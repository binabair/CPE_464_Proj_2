#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

#include "handleTable.h"

//struct for handle entries
//make the table
//add to table - check if resize is necessary (motherfuckerrrr thatll be two fxns)
//remove from table
//lookup - both of em

//struct
typedef struct {
    char handleName[100];
    int socketNum;
    int valid;
} HandleEntry;

typedef struct {
    HandleEntry *entries;
    int capacity;
    int count;
} HandleTable;

//make it
HandleTable createHandleTable(int size) {
    HandleTable currentTable;
    currentTable.capacity = size;
    currentTable.count = 0;

    currentTable.entries = malloc(size * sizeof(HandleEntry));
    if (currentTable.entries == NULL){
        perror("malloc failed");
        exit(1);
    }

    //initialize to prevent weird stuff
    for (int i = 0; i < currentTable.capacity; i++){
        currentTable.entries[i].valid = 0;
        currentTable.entries[i].socketNum = -1;
        currentTable.entries[i].handleName[0] = '\0';
    }

    return currentTable;
}

//add - returns -1 on failure, 0 on success
int addHandleEntry(char *handleName, int socketNum){

    //check if handle already exists - check by name cause socket nums are always unique
    for (int i=0; i < globalTable.count; i++){
        if ((strcmp(globalTable.entries[i].handleName, handleName) == 0) && (globalTable.entries[i].valid == 1)){
            printf("Handle already in use: %s\n", handleName);
            return -1;
        }
    }

    //check length - if not enough room, call expand and double size
    if (globalTable.count == globalTable.capacity){
        int expansion = expandHandleTable();
        if (expansion == -1){
            return -1;
        }
    }

    //put in new entry, increment count and return
    strncpy(globalTable.entries[globalTable.count].handleName, handleName, 99);
    globalTable.entries[globalTable.count].handleName[99] = '\0';
    globalTable.entries[globalTable.count].socketNum = socketNum;
    globalTable.entries[globalTable.count].valid = 1;
    globalTable.count ++;

    return 0;
}

//resize
int expandHandleTable(){
    int newCapacity = globalTable.capacity *2;

    HandleEntry *temp = realloc(globalTable.entries, newCapacity * sizeof(HandleEntry));
    if (temp == NULL){
        perror("realloc failed, sincerly the expandHandleTable fxn");
        return -1;
    }
    
    globalTable.entries = temp;

    //prevent garbage values
    for (int i = globalTable.capacity; i < newCapacity; i++){
        globalTable.entries[i].valid = 0;
        globalTable.entries[i].socketNum = -1;
        globalTable.entries[i].handleName[0] = '\0';
    }

    globalTable.capacity = newCapacity;
    return 0;
}    

//remove from table by socketNum
int removeHandleBySocket(int socketNum){
    for (int i = 0; i < globalTable.count; i++){
        if ((globalTable.entries[i].valid == 1) &&
            (globalTable.entries[i].socketNum == socketNum)){
            globalTable.entries[i].valid = 0;
            globalTable.entries[i].socketNum = -1;
            globalTable.entries[i].handleName[0] = '\0';
            return 0;
        }
    }
    return -1;
}



//lookup handle name
bool lookUpHandle (char * handleName){
    for (int i = 0; i < globalTable.count; i++){
        if ((strcmp(globalTable.entries[i].handleName, handleName) == 0) && (globalTable.entries[i].valid == 1)){
            return true;
        }
    }
    return false;
}

//lookup socket number
bool lookUpSocket (int socketNum){
    if(!globalTable.entries){
        createHandleTable(socketNum);
    }
    
    for (int i = 0; i < globalTable.count; i++){
        if ((globalTable.entries[i].socketNum == socketNum) && (globalTable.entries[i].valid == 1)){
            return true;
        }
    }
    return false;
}

//getting th handle by index for %L
char *getHandleByIndex(int index){
    if (index < 0 || index >= globalTable.count){
        return NULL;
    }

    if (globalTable.entries[index].valid == 1){
        return globalTable.entries[index].handleName;
    }

    return NULL;
}

//getting the valid handles for %L
int getHandleCount(){
    int count = 0;

    for (int i = 0; i < globalTable.count; i++){
        if (globalTable.entries[i].valid == 1){
            count++;
        }
    }

    return count;
}

//for broadcast
int getSocketByIndex(int index){
    if (index < 0 || index >= globalTable.count){
        return -1;
    }

    if (globalTable.entries[index].valid == 1){
        return globalTable.entries[index].socketNum;
    }

    return -1;
}

//get sockets from handles
int getSocketByHandle(char *handleName){
    for (int i = 0; i < globalTable.count; i++){
        if ((globalTable.entries[i].valid == 1) &&
            (strcmp(globalTable.entries[i].handleName, handleName) == 0)){
            return globalTable.entries[i].socketNum;
        }
    }
    return -1;
}



