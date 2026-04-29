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
#include "handleTable.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

void sendToServer(char * handleName, int socketNum);
int readFromStdin(uint8_t * buffer);
void checkArgs(int argc, char * argv[]);


void sendToServer(char * handleName, int socketNum)
{
	uint8_t buffer[MAXBUF];   //data buffer
	int stdinLen = 0;        //amount of data to send
	int recvBytes = 0;
	int enterHandle;

	//make handle table and add socket and handle
	HandleTable table1 = createHandleTable(256);
	if ((enterHandle = addHandleEntry(handleName, socketNum, table1)) == -1){
		perror("error adding handle");
	}
	
	stdinLen = readFromStdin(buffer);
	//printf("read: %s string len: %d (including null)\n", buffer, sendLen);

	//string of if statements pulled from the depths
	if((buffer[1] == 'm') || (buffer[1] == 'M')){ //%M handle1 Hello how are you
		mCall(buffer, &table1);
	}else if ((buffer[1] == 'b') || (buffer[1] == 'B')){
		bCall();
	}else if ((buffer[1] == 'c') || (buffer[1] == 'C')){
		cCall();
	}else if ((buffer[1] == 'l') || (buffer[1] == 'L')){
		lCall(&table1);
	}
	
	//sent = sendPDU(socketNum, buffer, sendLen);

	// sent =  safeSend(socketNum, buffer, sendLen, 0);
	// if (sent < 0)
	// {
	// 	perror("send call");
	// 	exit(-1);
	// }

	// printf("Socket:%d: Sent, Length: %d msg: %s\n", socketNum, sent, buffer);
	
	// just for debugging, recv a message from the server to prove it works.
	recvBytes = recvPDU(socketNum, buffer, MAXBUF);
	if (recvBytes == 0){
		printf("Server has terminated\n");
		exit(-1);
	}else if (recvBytes < 0){
		perror("recv call");
		exit(-1);
	} else if (recvBytes > 0){
		printf("Socket %d: Byte recv: %d message: %s\n", socketNum, recvBytes, buffer);
	}
	
}

//%M - send to specific destination, %B - broadcast, %C - send to some, not all,
//%L - list all handlesknown by server




int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;  
	
	//do print the $: first, then call poll cause poll with take over everything and not let it print
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	printf("$: "); //fancy fancy
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
	if (argc != 4) //up this to accomodate new run command
	{
		printf("usage: %s host-name port-number \n", argv[0]);
		exit(1);
	}
}

int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);
	
	while(1){
		sendToServer(argv[1], socketNum);
	}
	
	close(socketNum);
	
	return 0;
}
