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

#define MAXBUF 1024
#define DEBUG_FLAG 1

void sendToServer(int socketNum);
int readFromStdin(uint8_t * buffer);
void checkArgs(int argc, char * argv[]);
void clientControl(int socketNum, char * myHandle);
void processMsgFromServer(int socketNum);
void processStdin(int socketNum, char * myHandle);




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
    int flag;

    if (recvBytes == 0){
        printf("Server has terminated\n");
        exit(-1);
    }else if (recvBytes < 0){
        perror("recv call");
        exit(-1);
    } else if (recvBytes > 0){
        //parse and print incoming message 
        // should be just flag from server
        flag = buffer[0];
        if (flag == 2){

        }
        //printf("Socket %d: Byte recv: %d message: %s\n", socketNum, recvBytes, buffer);
    }


}

void processStdin(int socketNum, char * myHandle){
    uint8_t buffer[MAXBUF];   //data buffer
    int sendLen = 0;        //amount of data to send
    int sent = 0;            //actual amount of data sent

    sendLen = readFromStdin(buffer);

    //string of if statements pulled from the depths
	if((buffer[1] == 'm') || (buffer[1] == 'M')){ //%M handle1 Hello how are you
		mCall(buffer, socketNum, myHandle);    //put message together and send to server in each one in the specific way that it needs to 

	}else if ((buffer[1] == 'b') || (buffer[1] == 'B')){
		bCall();
	}else if ((buffer[1] == 'c') || (buffer[1] == 'C')){
		cCall(buffer, socketNum, myHandle);
	}else if ((buffer[1] == 'l') || (buffer[1] == 'L')){
		lCall(globalTable);
	}


    
    //sendLen = readFromStdin(buffer);
    //printf("read: %s string len: %d (including null)\n", buffer, sendLen);
    
    if (sent < 0)
    {
        perror("send call");
        exit(-1);
    }

    printf("Socket:%d: Sent, Length: %d msg: %s\n", socketNum, sent, buffer);
    
}

//%M - send to specific destination, %B - broadcast, %C - send to some, not all,
//%L - list all handlesknown by server

void mCall(char * input, int serverSocket, char * myHandle){
	char destHandle[100];
    char message[200];
    uint8_t payload[512];

    int myHandleLen = strlen(myHandle);
    int destLen = 0;
    int msgLen = 0;
    int index = 0;

    int i = 2;  // skip %M, also set i for the loops

    for (; input[i] == ' '; i++);

    // parse destination handle
    for (; input[i] != ' ' && input[i] != '\0' && destLen < 99; i++) {
        destHandle[destLen++] = input[i];
    }
    destHandle[destLen] = '\0';

    if (input[i] == ' ') {
        i++;
    }

    // get message
    for (; input[i] != '\0' && msgLen < 199; i++) {
        message[msgLen++] = input[i];
    }
    message[msgLen] = '\0';

    //flag (5) + length of sending clients handle + sending clients handle itself + literal number 1 (duh that why we %M in the first place)
    //+ destination handle (1byte handle len +handle name) + message
    //this gets back flag 7 if handle dos not exist

    payload[index++] = 5; // flag

    payload[index++] = (uint8_t) myHandleLen; // sender handle length and make it exactly one byte
    for (int j = 0; j < myHandleLen; j++) { //sender handle
        payload[index++] = myHandle[j];
    }

    payload[index++] = 1; // number of destination handles

    payload[index++] = (uint8_t) destLen; // dest handle length (make it one byte)
    for (int j = 0; j < destLen; j++) {  //copy over dest handle
        payload[index++] = destHandle[j];
    }

    for (int j = 0; j <= msgLen; j++) {  //copy over message 
        payload[index++] = message[j];
    }

    // send it
    if (sendPDU(serverSocket, payload, index) < 0) {
        perror("sendPDU");
        exit(1);
    }
	//printf("Socket:%d: Sent, Length: %d msg: %s\n", socketNum, sent, buffer);

}

void bCall(){

}

void cCall(char *input, int serverSocket, char *myHandle){
    char destHandles[9][100];
    char message[200];
    uint8_t payload[1024];

    int myHandleLen = strlen(myHandle);
    int numHandles = 0;
    int destLens[9];
    int msgLen = 0;
    int index = 0;
    int i = 2;   // skip %C and start i

    for ( ; input[i] == ' '; i++);

    if (input[i] >= '2' && input[i] <= '9') {  // mkae sure num of handles is within range
        numHandles = input[i] - '0';
        i++;
    } else {
        printf("Invalid number of handles for %%C\n");
        return;
    }

    for ( ; input[i] == ' '; i++);

    // parse each destination handle
    for (int h = 0; h < numHandles; h++) {
        int len = 0;

        for ( ; input[i] != ' ' && input[i] != '\0' && len < 99; i++) { //drop each one in at correct handle, int the array of them
            destHandles[h][len] = input[i];
            len++;
        }
        destHandles[h][len] = '\0';
        destLens[h] = len; //add the length of each handle respectively

        if (len == 0) {  //some bullshit happened
            printf("Missing destination handle in %%C\n"); 
            return;
        }

        if (h < numHandles - 1) {
            if (input[i] == '\0') {
                printf("Not enough destination handles for %%C\n");
                return;
            }

            for ( ; input[i] == ' '; i++);
        }
    }

    // skip spaces
    for ( ; input[i] == ' '; i++);

    // parse message
    for ( ; input[i] != '\0' && msgLen < 199; i++) {
        message[msgLen] = input[i];
        msgLen++;
    }
    message[msgLen] = '\0';

    // build payload same as %m
    payload[index] = 6;  index++; // multicast flag
    
    payload[index] = (uint8_t) myHandleLen; index++;

    for (int j = 0; j < myHandleLen; j++) {
        payload[index] = myHandle[j];
        index++;
    }

    payload[index] = (uint8_t) numHandles;
    index++;

    for (int h = 0; h < numHandles; h++) {
        payload[index] = (uint8_t) destLens[h];
        index++;

        for (int j = 0; j < destLens[h]; j++) {
            payload[index] = destHandles[h][j];
            index++;
        }
    }

    // copy message including null terminator
    for (int j = 0; j <= msgLen; j++) {
        payload[index] = message[j];
        index++;
    }

    if (sendPDU(serverSocket, payload, index) < 0) {
        perror("sendPDU");
        exit(1);
    }
}

void lCall(HandleTable *table){


}


int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;     
    
    //do print the $: first, then call poll cause poll with take over everything and not let it print
	
	printf("$: "); //fancy fancy
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	printf("Enter data: ");
	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s host-name port-number \n", argv[0]);
		exit(1);
	}
}


void initClient(int socketNum, int handleLen, char * handle){
    char data[2+handleLen]; //flag + handle len + handle
    data[0] = 1;
    data[1] = handleLen;
    int handleIndex = 0;

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
        printf("Server has terminated\n");
        exit(-1);
    }

    int flag = buffer[0];

    if (flag == 3){
        printf("Server rejected initial handle");
        close(socketNum);
        exit(-1);
    }else if (flag == 2){
        addHandleEntry(handle, socketNum);
    }
    

}


int main(int argc, char * argv[]){
	int socketNum = 0; //socket descriptor
	checkArgs(argc, argv);

    char myHandle[100] = strcpy(myHandle, argv[1]);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);

    setupPollSet();
    addToPollSet(socketNum); //the server

    initClient(socketNum, strlen(myHandle), myHandle);

    clientControl(socketNum, myHandle);
	
	return 0;
}