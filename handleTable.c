#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "handleTable.h"

//struct for handle entries
//make the table
//add to table - check if resize is necessary (motherfuckerrrr thatll be two fxns)
//remove from table

//struct
typedef struct {
    char * handleName;
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

    currentTable.entries = malloc(size * sizeof(HandleEntry));
    //initialize to prevent weird stuff
    for (int i = 0; i < currentTable.capacity; i++){
        currentTable.entries[i].valid = 0;
        currentTable.entries[i].socketNum = -1;
    }

    currentTable.capacity = size;
    currentTable.count = 0;
    return currentTable;
}

//add
int addHandleEntry(HandleEntry handle, HandleTable *table){

    //check if handle already exists - check by name cause socket nums are always unique
    for (int i=0; i < table->count; i++){
        if (table->entries[i].handleName == handle.handleName){
            printf("Handle already in use: %s", handle.handleName);
            return -1;
        }
    }

    //check length - if not enough room, call expand and double size
    if (table->count == table->capacity){
        int expansion = expandHandleTable(table);
        if (expansion == -1){
            perror("realloc failed, sincerely the addHandleEntry fxn");
        }
    }

    //put in new entry, increment count and return
    table->entries[table->count + 1] = handle;
    table->count += 1;

    return table;
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
    }

    table->capacity = newCapacity;
    return 0;
}    


