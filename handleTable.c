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
//add to table - check if resize is necessary 
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
    //check length
    if ()
}


