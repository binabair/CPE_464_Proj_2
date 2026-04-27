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

//add
int addHandleEntry(char *handleName, int socketNum, HandleTable *table){

    //check if handle already exists - check by name cause socket nums are always unique
    for (int i=0; i < table->count; i++){
        if ((strcmp(table->entries[i].handleName, handleName) == 0) && (table->entries[i].valid == 1)){
            printf("Handle already in use: %s\n", handleName);
            return -1;
        }
    }

    //check length - if not enough room, call expand and double size
    if (table->count == table->capacity){
        int expansion = expandHandleTable(table);
        if (expansion == -1){
            return -1;
        }
    }

    //put in new entry, increment count and return
    strncpy(table->entries[table->count].handleName, handleName, 99);
    table->entries[table->count].handleName[99] = '\0';
    table->entries[table->count].socketNum = socketNum;
    table->entries[table->count].valid = 1;
    table->count ++;

    return 0;
}

//resize
int expandHandleTable(HandleTable *table){
    int newCapacity = table->capacity *2;

    HandleEntry *temp = realloc(table->entries, newCapacity * sizeof(HandleEntry));
    if (temp == NULL){
        perror("realloc failed, sincerly the expandHandleTable fxn");
        return -1;
    }
    
    table->entries = temp;

    //prevent garbage values
    for (int i = table->capacity; i < newCapacity; i++){
        table->entries[i].valid = 0;
        table->entries[i].socketNum = -1;
        table->entries[i].handleName[0] = '\0';
    }

    table->capacity = newCapacity;
    return 0;
}    

//remove from table
int removeHandleEntry(HandleEntry handle, HandleTable *table){
    for (int i = 0; i < table->count; i++){
        if (table->entries[i].socketNum == handle.socketNum){
            table->entries[i].valid = 0;
            table->entries[i].socketNum = -1;
            table->entries[i].handleName[0] = '\0';
            return 0;
        }
    }
    return -1;
}

//lookup handle name
bool lookUpHandle (char * handleName, HandleTable *table){
    for (int i = 0; i < table->count; i++){
        if ((strcmp(table->entries[i].handleName, handleName) == 0) && (table->entries[i].valid == 1)){
            return true;
        }
    }
    return false;
}

//lookup socket number
bool lookUpSocket (int socketNum, HandleTable *table){
    for (int i = 0; i < table->count; i++){
        if ((table->entries[i].socketNum == socketNum) && (table->entries[i].valid == 1)){
            return true;
        }
    }
    return false;
}

