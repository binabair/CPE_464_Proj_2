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
void clientControl(int socketNum);
void processMsgFromServer(int socketNum);
void processStdin(int socketNum);




void clientControl(int serverSocket){
    addToPollSet(STDIN_FILENO); //standard in

    // wait for client to connect
    while (1){
        int readySocket = pollCall(-1); //block until a socket is ready for read

        if (readySocket == serverSocket){
            processMsgFromServer(serverSocket);
        }
        else if (readySocket == STDIN_FILENO){
            processStdin(serverSocket);
        }
    }
}

void processMsgFromServer(int socketNum){
    uint8_t buffer[MAXBUF];
    int recvBytes = recvPDU(socketNum, buffer, MAXBUF);

    if (recvBytes == 0){
        printf("Server has terminated\n");
        exit(-1);
    }else if (recvBytes < 0){
        perror("recv call");
        exit(-1);
    } else if (recvBytes > 0){
        //parse and print incoming message

        buffer[]
        printf("Socket %d: Byte recv: %d message: %s\n", socketNum, recvBytes, buffer);
    }


}

void processStdin(int socketNum){
    uint8_t buffer[MAXBUF];   //data buffer
    int sendLen = 0;        //amount of data to send
    int sent = 0;            //actual amount of data sent


        //string of if statements pulled from the depths
	if((buffer[1] == 'm') || (buffer[1] == 'M')){ //%M handle1 Hello how are you
		mCall(buffer, &table1);    //put message together and send to server in each one in the specific way that it needs to 

	}else if ((buffer[1] == 'b') || (buffer[1] == 'B')){
		bCall();
	}else if ((buffer[1] == 'c') || (buffer[1] == 'C')){
		cCall();
	}else if ((buffer[1] == 'l') || (buffer[1] == 'L')){
		lCall(&table1);
	}


    
    //sendLen = readFromStdin(buffer);
    //printf("read: %s string len: %d (including null)\n", buffer, sendLen);
    
    sent = sendPDU(socketNum, buffer, sendLen);

    if (sent < 0)
    {
        perror("send call");
        exit(-1);
    }

    printf("Socket:%d: Sent, Length: %d msg: %s\n", socketNum, sent, buffer);
    
}

//%M - send to specific destination, %B - broadcast, %C - send to some, not all,
//%L - list all handlesknown by server

void mCall(char buffer[350]){
	char command[3];
	char handleName[100];
	char * message[200];
	int sent = 0;

	int i = 2; // skip m
    int j = 0;

	//parse this bitch - itll be put in perfectly

	while (buffer[i] != ' ' && buffer[i] != '\0') {
        handleName[j++] = buffer[i++];
    }

	handleName[j] = '\0'; //terminate the handlename

	if (buffer[i] == ' ') { //check if theres message or not
		i++;
	}
    // If no message - empty string
    if (buffer[i] == '\0') {
        message[0] = '\0';
    } else {
        int k = 0;
        while (buffer[i] != '\0') {
            message[k++] = buffer[i++];
        }
        message[k] = '\0';
    }

	//must attach message packet length (2bytes)
	sent = sendPDU(socketNum, buffer, sendLen);

	sent =  safeSend(socketNum, buffer, sendLen, 0);
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("Socket:%d: Sent, Length: %d msg: %s\n", socketNum, sent, buffer);


}

void bCall(){

}

void cCall(){

}

void lCall(HandleTable *table){
	//
	for (int i = 0; i < table->count; i++){
		//Give this to Flag = 12, whever the hell that means
	}


}


int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;     
    
    //do print the $: first, then call poll cause poll with take over everything and not let it print
	
	// Important you don't input more characters than you have space 
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

    for (int i = 2; i < (handleLen + 2); i++){
        data[i] = handle[handleIndex];
    }

    sendPDU(socketNum, data, (2+handleLen));

    int serverReply = pollCall(-1);
    processMsgFromServer(serverReply);

}


int main(int argc, char * argv[]){
	int socketNum = 0; //socket descriptor
    int enterHandle;
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);

    setupPollSet();
    addToPollSet(socketNum); //the server

    initClient(socketNum, sizeof(argv[1]), argv[1]);

    clientControl(socketNum);
	
	return 0;
}