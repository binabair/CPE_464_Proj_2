/******************************************************************************
* myClient.c
*
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include <ctype.h>

#include "networks.h"
#include "safeUtil.h"
#include "sendRecvPDU.h"
#include "pollLib.h"
#include "handleTable.h"

#define MAXBUF 1401
#define DEBUG_FLAG 1
#define MAX_HANDLE 100

int listMode = 0;
int listCountRemaining = 0;

void sendToServer(int socketNum);
int readFromStdin(uint8_t * buffer);
void checkArgs(int argc, char * argv[]);
void clientControl(int socketNum, char * myHandle);
void processMsgFromServer(int socketNum);
void processStdin(int socketNum, char * myHandle);
void mCall(char * input, int serverSocket, char * myHandle);
void cCall(char *input, int serverSocket, char *myHandle);
void bCall(char *input, int serverSocket, char *myHandle);
void lCall(int serverSocket);
void initClient(int socketNum, int handleLen, char * handle);
void sendMessagePDU(int socket, char *myHandle, char *destHandle, char *message);
void sendBroadcastPDU(int socket, char *myHandle, char *message);
void sendMulticastPDU(int socket, char *myHandle, int numHandles, char handles[][101], char *message);


void clientControl(int serverSocket, char * myHandle){
    addToPollSet(STDIN_FILENO); //standard in

    // wait for client to connect
    while (1){
        int readySocket = pollCall(-1); //block until a socket is ready for read

        if (readySocket == serverSocket){
            processMsgFromServer(serverSocket);
        }
        else if (readySocket == STDIN_FILENO){
            processStdin(serverSocket, myHandle);
        }
    }
}

void processMsgFromServer(int socketNum){
    uint8_t buffer[MAXBUF];
    int recvBytes = recvPDU(socketNum, buffer, MAXBUF);

    if (recvBytes == 0){
        printf("Server Terminated\n");
        exit(-1);
    }else if (recvBytes < 0){
        perror("recv call");
        exit(-1);
    } 

    uint8_t flag = buffer[0];

    switch(flag){
        case 4: {
            int index = 1;
            int senderLen = buffer[index];
            index++;

            char sender[256];
            for (int i = 0; i < senderLen; i++){
                sender[i] = buffer[index];
                index++;
            }
            sender[senderLen] = '\0';

            char *message = (char *)&buffer[index];

            printf("\n%s: %s\n", sender, message);
            printf("$: ");
            fflush(stdout);
            break;
        }

        case 5: {
            int index = 1;
            int senderLen = buffer[index];
            index++;

            char sender[256];
            for (int i = 0; i < senderLen; i++){
                sender[i] = buffer[index];
                index++;
            }
            sender[senderLen] = '\0';

            int destCount = buffer[index];
            index++;

            for (int h = 0; h < destCount; h++){
                int destLen = buffer[index];
                index++;

                for (int i = 0; i < destLen; i++){
                    index++;
                }
            }

            char *message = (char *)&buffer[index];

            printf("\n%s: %s\n", sender, message);
            printf("$: ");
            fflush(stdout);
            break;
        }

        case 6: {
            int index = 1;
            int senderLen = buffer[index];
            index++;

            char sender[256];
            for (int i = 0; i < senderLen; i++){
                sender[i] = buffer[index];
                index++;
            }
            sender[senderLen] = '\0';

            int destCount = buffer[index];
            index++;

            for (int h = 0; h < destCount; h++){
                int destLen = buffer[index];
                index++;

                for (int i = 0; i < destLen; i++){
                    index++;
                }
            }

            char *message = (char *)&buffer[index];

            printf("\n%s: %s\n", sender, message);
            printf("$: ");
            fflush(stdout);
            break;
        }

        case 7: {
            // error: destination handle does not exist
            int badHandleLen = buffer[1];
            char badHandle[256];

            for (int i = 0; i < badHandleLen; i++){
                badHandle[i] = buffer[i + 2];
            }
            badHandle[badHandleLen] = '\0';

            printf("Client with handle %s does not exist.\n", badHandle);
            printf("$: ");
            fflush(stdout);
            break;
        }

        case 11: {
            // response to %L: number of handles follows
            uint32_t numHandlesNet;
            uint32_t numHandles;

            memcpy(&numHandlesNet, &buffer[1], sizeof(uint32_t));
            numHandles = ntohl(numHandlesNet);

            listMode = 1;
            listCountRemaining = (int)numHandles;

            printf("Number of clients: %u\n", numHandles);
            break;
        }

        case 12: {
            // one handle from the server's handle list
            int handleLen = buffer[1];
            char handle[256];

            for (int i = 0; i < handleLen; i++){
                handle[i] = buffer[i + 2];
            }
            handle[handleLen] = '\0';

            printf("%s\n", handle);

            if (listMode && listCountRemaining > 0){
                listCountRemaining--;
            }
            break;
        }

        case 13:
            // %L finished
            listMode = 0;
            listCountRemaining = 0;
            printf("$: ");
            fflush(stdout);
            break;

        default:
            printf("Unknown flag %d received from server\n", flag);
            printf("$: ");
            fflush(stdout);
            break;
    }


}

void processStdin(int socketNum, char * myHandle){
    uint8_t buffer[MAXBUF];   //data buffer
    int sendLen = 0;        //amount of data to send

    sendLen = readFromStdin(buffer);

    if (sendLen <= 1){
        printf("Invalid command\n");
        printf("$: ");
        fflush(stdout);
        return;
    }
    if (buffer[0] != '%'){
        printf("Invalid command\n");
        printf("$: ");
        fflush(stdout);
        return;
    }

    //string of if statements pulled from the depths
	if((buffer[1] == 'm') || (buffer[1] == 'M')){ //%M handle1 Hello how are you
		mCall((char *)buffer, socketNum, myHandle);
	}
    else if ((buffer[1] == 'b') || (buffer[1] == 'B')){
		bCall((char *)buffer, socketNum, myHandle);
	}
    else if ((buffer[1] == 'c') || (buffer[1] == 'C')){
		cCall((char *)buffer, socketNum, myHandle);
	}
    else if ((buffer[1] == 'l') || (buffer[1] == 'L')){
		lCall(socketNum);
	}
    else {
    printf("Invalid command\n");
    printf("$: ");
    fflush(stdout);
    }  
}

//%M - send to specific destination, %B - broadcast, %C - send to some, not all,
//%L - list all handlesknown by server

void mCall(char * input, int serverSocket, char * myHandle){
	char destHandle[101];
    char message[MAXBUF];
    int msgLen = 0;
    int i = 2;  // skip %M, also set i for the loops

    for (; input[i] == ' '; i++);
    
    // parse destination handle with length check
    int tempLen = 0;
    int start = i;

    // first pass: measure full handle length
    while (input[i] != ' ' && input[i] != '\0') {
        tempLen++;
        i++;
    }

    if (tempLen > MAX_HANDLE) {
        printf("Invalid handle, handle longer than 100 characters\n");
        printf("$: ");
        fflush(stdout);
        return;
    }

    if (tempLen == 0) {
        printf("Invalid command format\n");
        printf("$: ");
        fflush(stdout);
        return;
    }

    // second pass: copy safely
    for (int j = 0; j < tempLen; j++) {
        destHandle[j] = input[start + j];
    }
    destHandle[tempLen] = '\0';

    for (; input[i] == ' '; i++);

    // get message
    for (; input[i] != '\0'; i++) {
        message[msgLen++] = input[i];
    }
    message[msgLen] = '\0';

    // Split into multiple packets if needed
    if (msgLen == 0) {
        sendMessagePDU(serverSocket, myHandle, destHandle, "");
        return;
    }

    char chunk[200];

    for (int offset = 0; offset < msgLen; offset += 199) {
        int chunkLen = msgLen - offset;
        if (chunkLen > 199) {
            chunkLen = 199;
        }

        memcpy(chunk, message + offset, chunkLen);
        chunk[chunkLen] = '\0';

        sendMessagePDU(serverSocket, myHandle, destHandle, chunk);
    }
}

void bCall(char *input, int serverSocket, char *myHandle){
    char message[MAXBUF];
    int msgLen = 0;
    int i = 2;   // skip %B or %b

    // skip spaces after %B
    for ( ; input[i] == ' '; i++) {
    }

    // parse message
    for ( ; input[i] != '\0'; i++) {
        message[msgLen] = input[i];
        msgLen++;
    }
    message[msgLen] = '\0';

    // send empty broadcast message if no text
    if (msgLen == 0) {
        sendBroadcastPDU(serverSocket, myHandle, "");
        return;
    }

    char chunk[200];

    for (int offset = 0; offset < msgLen; offset += 199) {
        int chunkLen = msgLen - offset;
        if (chunkLen > 199) {
            chunkLen = 199;
        }

        memcpy(chunk, message + offset, chunkLen);
        chunk[chunkLen] = '\0';

        sendBroadcastPDU(serverSocket, myHandle, chunk);
    }
}

void cCall(char *input, int serverSocket, char *myHandle){
    char destHandles[9][101];
    char message[MAXBUF];
    int numHandles = 0;
    int msgLen = 0;
    int i = 2;   // skip %C and start i

    for ( ; input[i] == ' '; i++);

    if (input[i] >= '2' && input[i] <= '9') {  // mkae sure num of handles is within range
        numHandles = input[i] - '0';
        i++;
    } else {
        printf("Invalid number of handles for %%C\n");
        printf("$: ");
        fflush(stdout);
        return;
    }

    for ( ; input[i] == ' '; i++);

    // parse each destination handle
    for (int h = 0; h < numHandles; h++) {
        int tempLen = 0;
        int start = i;

        // measure full handle length
        while (input[i] != ' ' && input[i] != '\0') {
            tempLen++;
            i++;
        }

        if (tempLen > MAX_HANDLE) {
            printf("Invalid handle, handle longer than 100 characters\n");
            printf("$: ");
            fflush(stdout);
            return;
        }

        if (tempLen == 0) {
            printf("Missing destination handle in %%C\n");
            printf("$: ");
            fflush(stdout);
            return;
        }

        // copy handle
        for (int j = 0; j < tempLen; j++) {
            destHandles[h][j] = input[start + j];
        }
        destHandles[h][tempLen] = '\0'; //add in len and the null pointers

        if (h < numHandles - 1) {
            if (input[i] == '\0') {
                printf("Not enough destination handles for %%C\n");
                printf("$: ");
                fflush(stdout);
                return;
            }

            for ( ; input[i] == ' '; i++);
        }
    }

    // skip spaces
    for ( ; input[i] == ' '; i++);

    // parse full message
    for ( ; input[i] != '\0'; i++) {
        message[msgLen] = input[i];
        msgLen++;
    }
    message[msgLen] = '\0';

    // send empty multicast message if no text
    if (msgLen == 0) {
        sendMulticastPDU(serverSocket, myHandle, numHandles, destHandles, "");
        return;
    }

    char chunk[200];

    for (int offset = 0; offset < msgLen; offset += 199) {
        int chunkLen = msgLen - offset;
        if (chunkLen > 199) {
            chunkLen = 199;
        }

        memcpy(chunk, message + offset, chunkLen);
        chunk[chunkLen] = '\0';

        sendMulticastPDU(serverSocket, myHandle, numHandles, destHandles, chunk);
    }
}

void lCall(int serverSocket){
    uint8_t payload[1];

    payload[0] = 10;   // flag for %L request

    if (sendPDU(serverSocket, payload, 1) < 0) {
        perror("sendPDU");
        exit(1);
    }
}

void sendMessagePDU(int socket, char *myHandle, char *destHandle, char *message) {
    uint8_t pdu[MAXBUF];
    int idx = 0;

    pdu[idx++] = 5; // flag

    uint8_t senderLen = strlen(myHandle);
    pdu[idx++] = senderLen;
    memcpy(pdu + idx, myHandle, senderLen);
    idx += senderLen;

    pdu[idx++] = 1; // one destination

    uint8_t destLen = strlen(destHandle);
    pdu[idx++] = destLen;
    memcpy(pdu + idx, destHandle, destLen);
    idx += destLen;

    strcpy((char *)(pdu + idx), message);
    idx += strlen(message) + 1;

    sendPDU(socket, pdu, idx);
}

void sendBroadcastPDU(int socket, char *myHandle, char *message) {
    uint8_t pdu[MAXBUF];
    int idx = 0;

    pdu[idx++] = 4; // flag

    uint8_t senderLen = strlen(myHandle);
    pdu[idx++] = senderLen;
    memcpy(pdu + idx, myHandle, senderLen);
    idx += senderLen;

    strcpy((char *)(pdu + idx), message);
    idx += strlen(message) + 1;

    sendPDU(socket, pdu, idx);
}

void sendMulticastPDU(int socket, char *myHandle, int numHandles,
    char handles[][101], char *message) {
    uint8_t pdu[MAXBUF];
    int idx = 0;

    pdu[idx++] = 6; // flag

    uint8_t senderLen = strlen(myHandle);
    pdu[idx++] = senderLen;
    memcpy(pdu + idx, myHandle, senderLen);
    idx += senderLen;

    pdu[idx++] = numHandles;

    for (int i = 0; i < numHandles; i++) {
        uint8_t len = strlen(handles[i]);
        pdu[idx++] = len;
        memcpy(pdu + idx, handles[i], len);
        idx += len;
    }

    strcpy((char *)(pdu + idx), message);
    idx += strlen(message) + 1;

    sendPDU(socket, pdu, idx);
}


int readFromStdin(uint8_t * buffer)
{
	int aChar = 0;
	int inputLen = 0;     
    
    //do print the $: first, then call poll cause poll with take over everything and not let it print
	
	printf("$: "); //fancy fancy
    fflush(stdout);
	
	// Important you don't input more characters than you have space 
	while (inputLen < (MAXBUF - 1) && aChar != '\n' && aChar != EOF)
	{
		aChar = getchar();
		if (aChar != '\n' && aChar != EOF)
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';

    // flush extra input if too long
    if (aChar != '\n' && aChar != EOF) {
        while ((aChar = getchar()) != '\n' && aChar != EOF);
    }

	inputLen++;
	
	return inputLen;
}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s handle host-name port-number \n", argv[0]);
		exit(1);
	}
    if ((int)strlen(argv[1]) > MAX_HANDLE) {
        fprintf(stderr, "Invalid handle, handle longer than 100 characters: %s\n", argv[1]);
        exit(1);
    }
}


void initClient(int socketNum, int handleLen, char * handle){
    char data[2+handleLen]; //flag + handle len + handle
    data[0] = 1;
    data[1] = handleLen;

    for (int i = 0; i < handleLen; i++){
        data[i+2] = handle[i];
    }

    if (sendPDU(socketNum, data, 2 + handleLen) < 0){
        perror("sendPDU");
        close(socketNum);
        exit(1);
    }

    uint8_t buffer[MAXBUF];
    int recvBytes = recvPDU(socketNum, buffer, MAXBUF);

    if (recvBytes == 0){
        printf("Server Terminated\n");
        exit(-1);
    }

    if (recvBytes < 0){
    perror("recv call");
    close(socketNum);
    exit(-1);
    }

    int flag = buffer[0];

    if (flag == 3){
        printf("Handle already in use: %s\n", handle);
        close(socketNum);
        exit(-1);
    }else if (flag == 2){
        addHandleEntry(handle, socketNum);
    }
    

}


int main(int argc, char * argv[]){
	int socketNum = 0; //socket descriptor
	checkArgs(argc, argv);
    char myHandle[101];
    strcpy(myHandle, argv[1]);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);
    setupPollSet();
    addToPollSet(socketNum); //the server

    initClient(socketNum, strlen(myHandle), myHandle);
    clientControl(socketNum, myHandle);
	
	return 0;
}