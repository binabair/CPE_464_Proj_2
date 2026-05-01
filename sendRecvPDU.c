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
#include <errno.h>

#include "sendRecvPDU.h"

//send 'er over
int sendPDU(int clientSocket, uint8_t * dataBuffer, int lengthOfData){ //returns the number of bytes sent
    int sent = 0; //actual amount of data sent
    int pduLength = lengthOfData + 2;
    uint8_t pduBuff[1402];

    if (pduLength > 1402) {
        fprintf(stderr, "PDU too large\n");
        return -1;
    }

    uint16_t netOrderLen = htons(pduLength); 
    memcpy(pduBuff, &netOrderLen, 2); 
    memcpy(pduBuff + 2, dataBuffer, lengthOfData);

    sent = send(clientSocket, pduBuff, pduLength, 0); //no special flags - will use MSG_WAITALL in recv()

    if (sent < 0){
        perror("send error");
        exit(-1);
    }

    return sent;
}

//recieve 'er 
int recvPDU(int socketNumber, uint8_t * dataBuffer, int bufferSize){
    uint8_t lenBuff[2]; 
    uint16_t netLen = 0;

    int recvLen = recv(socketNumber, lenBuff, 2, MSG_WAITALL); //wait for entire data

    if (recvLen == 0){
        //connection closed
        return 0;
    }

    if (recvLen < 0){
        if (errno == ECONNRESET){
            return 0;   // treat as closed
        }
        perror("recv error");
        return(-1);
    }

    memcpy(&netLen, lenBuff, 2);
    uint16_t totalPDULen = ntohs(netLen);

    int hostDataLength = totalPDULen - 2;

    if (hostDataLength > bufferSize){
        fprintf(stderr, "Buffer too small for received data\n");
        return(-1);
    }

    int recvData = recv(socketNumber, dataBuffer, hostDataLength, MSG_WAITALL);

    if (recvData == 0) {
        return 0;   // connection closed
    }
    if (recvData < 0) {
        if (errno == ECONNRESET){
            return 0;   // treat as closed
        }
        perror("recv");
        return -1;
    }

    return recvData;
}