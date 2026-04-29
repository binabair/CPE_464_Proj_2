/******************************************************************************
* myServer.c
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

#include "networks.h"
#include "safeUtil.h"
#include "sendRecvPDU.h"
#include "pollLib.h"
#include "handleTable.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

void recvFromClient(int clientSocket);
int checkArgs(int argc, char *argv[]);
void serverControl(int mainServerSocket);
void addNewSocket(int mainServerSocket);
void processClient(int clientSocket);

int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	// int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;
	
	portNumber = checkArgs(argc, argv);
	
	//create the server socket
	mainServerSocket = tcpServerSetup(portNumber);

    serverControl(mainServerSocket);

	return 0;
}


void serverControl(int mainServerSocket){
    int clientSocket = 0;   //socket descriptor for the client socket
    setupPollSet();
    addToPollSet(mainServerSocket);

    // wait for client to connect
    while (1){
        clientSocket = pollCall(-1); //block until a socket is ready for read

        if (clientSocket == mainServerSocket){
            //new client connection
            addNewSocket(mainServerSocket);
        }
        else{
            //data from an existing client
            processClient(clientSocket);
        }
    }
}

void addNewSocket(int mainServerSocket){
    bool existingSocket;
    int clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);
    addToPollSet(clientSocket);
    existingSocket = lookUpSocket(clientSocket);
    if (existingSocket == false) {
        addHandleEntry()
    }

}

void processClient(int clientSocket){
    uint8_t dataBuffer[MAXBUF];
    int recvOut = 0;
    int messageLen = 0;
    
    //now get the data from the client_socket
    recvOut = recvPDU(clientSocket, dataBuffer, MAXBUF);

    if (recvOut > 0)
    {
        printf("Socket %d: Message received, length: %d Data: %s\n", clientSocket, recvOut, dataBuffer);
        
        // send it back to client (just to test sending is working... e.g. debugging)
        messageLen = sendPDU(clientSocket, dataBuffer, recvOut);
        printf("Socket %d: msg sent: %d bytes, text: %s\n", clientSocket, messageLen, dataBuffer);
    }
    else if (recvOut == 0)
    {
        printf("Socket %d: Connection closed by other side\n", clientSocket);
        removeFromPollSet(clientSocket);
        close(clientSocket);
    }else{
        perror("recv call");
        removeFromPollSet(clientSocket);
        close(clientSocket);
    }
}


int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

