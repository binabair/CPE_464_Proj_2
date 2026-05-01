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
void processFlag1(int clientSocket, uint8_t *dataBuffer, int recvLen);
void processFlag4(int clientSocket, uint8_t *dataBuffer, int recvLen);
void processFlag5(int clientSocket, uint8_t *dataBuffer, int recvLen);
void processFlag6(int clientSocket, uint8_t *dataBuffer, int recvLen);
void processFlag10(int clientSocket);
void sendFlag7(int clientSocket, char *badHandle);


int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	// int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;
	
	portNumber = checkArgs(argc, argv);
	
    initHandleTable(10);
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
    int clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);
    //also parse that new shit for the handle name
    addToPollSet(clientSocket);
}

void processClient(int clientSocket){
    uint8_t dataBuffer[MAXBUF];
    int recvOut = 0;

    //now get the data from the client_socket
    recvOut = recvPDU(clientSocket, dataBuffer, MAXBUF);

    //parse the shit out of databuffer to get the handle name

    if (recvOut > 0){
        uint8_t flag = dataBuffer[0];

        switch(flag){
            case 1:
                processFlag1(clientSocket, dataBuffer, recvOut);
                break;

            case 4:
                processFlag4(clientSocket, dataBuffer, recvOut);
                break;

            case 5:
                processFlag5(clientSocket, dataBuffer, recvOut);
                break;

            case 6:
                processFlag6(clientSocket, dataBuffer, recvOut);
                break;

            case 10:
                processFlag10(clientSocket);
                break;

            default:
                printf("Unknown flag %d from socket %d\n", flag, clientSocket);
                break;
        }
    }
    else if (recvOut == 0) {
        printf("Socket %d: Connection closed by other side\n", clientSocket);
        removeFromPollSet(clientSocket);
        removeHandleBySocket(clientSocket);
        close(clientSocket);
    }else{
        perror("recv call");
        removeFromPollSet(clientSocket);
        removeHandleBySocket(clientSocket);
        close(clientSocket);
    }
}

void processFlag1(int clientSocket, uint8_t *dataBuffer, int recvLen){
    int handleLen = dataBuffer[1];
    char handle[256];

    for (int i = 0; i < handleLen; i++){
        handle[i] = dataBuffer[i + 2];
    }
    handle[handleLen] = '\0';

    if (lookUpHandle(handle) == true){
        uint8_t reply[1];
        reply[0] = 3;
        sendPDU(clientSocket, reply, 1);
    } else{
        addHandleEntry(handle, clientSocket);

        uint8_t reply[1];
        reply[0] = 2;
        sendPDU(clientSocket, reply, 1);
    }
}

void processFlag4(int clientSocket, uint8_t *dataBuffer, int recvLen){ 
    int validCount = getHandleCount();

    for(int i = 0; i < validCount; i++){
        int otherSocket = getNumberedValidSocket(i);

        if (otherSocket != -1 && otherSocket != clientSocket){
            sendPDU(otherSocket, dataBuffer, recvLen);
        }
    }
}

void processFlag5(int clientSocket, uint8_t *dataBuffer, int recvLen){
    int index = 1;

    int senderLen = dataBuffer[index];
    index++;

    char sender[256];
    for (int i = 0; i < senderLen; i++){
        sender[i] = dataBuffer[index];
        index++;
    }
    sender[senderLen] = '\0';

    int destCount = dataBuffer[index];
    index++;

    int destLen = dataBuffer[index];
    index++;

    char destHandle[256];
    for (int i = 0; i < destLen; i++){
        destHandle[i] = dataBuffer[index];
        index++;
    }
    destHandle[destLen] = '\0';

    int destSocket = getSocketByHandle(destHandle);

    if (destSocket < 0){
        sendFlag7(clientSocket, destHandle);
        return;
    }

    sendPDU(destSocket, dataBuffer, recvLen);
}

void processFlag6(int clientSocket, uint8_t *dataBuffer, int recvLen){
    int index = 1;

    int senderLen = dataBuffer[index];
    index++;

    char sender[256];
    for (int i = 0; i < senderLen; i++){
        sender[i] = dataBuffer[index];
        index++;
    }
    sender[senderLen] = '\0';

    int destCount = dataBuffer[index];
    index++;

    for (int h = 0; h < destCount; h++){
        int destLen = dataBuffer[index];
        index++;

        char destHandle[256];
        for (int i = 0; i < destLen; i++){
            destHandle[i] = dataBuffer[index];
            index++;
        }
        destHandle[destLen] = '\0';

        int destSocket = getSocketByHandle(destHandle);

        if (destSocket < 0){
            sendFlag7(clientSocket, destHandle);
        }
        else{
            sendPDU(destSocket, dataBuffer, recvLen);
        }
    }
}

void sendFlag7(int clientSocket, char *badHandle){
    int handleLen = strlen(badHandle);
    uint8_t reply[256];

    reply[0] = 7;
    reply[1] = handleLen;

    for (int i = 0; i < handleLen; i++){
        reply[i + 2] = badHandle[i];
    }

    sendPDU(clientSocket, reply, handleLen + 2);
}

void processFlag10(int clientSocket){
    uint8_t reply11[5];
    uint32_t count = getHandleCount();
    uint32_t countNet = htonl(count);

    reply11[0] = 11;
    memcpy(&reply11[1], &countNet, sizeof(uint32_t));
    sendPDU(clientSocket, reply11, 5);

    for (int i = 0; i < (int)count; i++){
        char *handle = getNumberedValidHandle(i);
        if (handle == NULL){
            continue;
        }

        int handleLen = strlen(handle);

        uint8_t reply12[256];
        reply12[0] = 12;
        reply12[1] = handleLen;

        for (int j = 0; j < handleLen; j++){
            reply12[j + 2] = handle[j];
        }

        sendPDU(clientSocket, reply12, handleLen + 2);
    }

    uint8_t reply13[1];
    reply13[0] = 13;
    sendPDU(clientSocket, reply13, 1);
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

