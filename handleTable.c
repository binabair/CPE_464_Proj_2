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
struct handleEntry {
    char * handleName;
    int socketNum;
    int valid;
};

//make it
void createHandleTable(struct handleEntry firstHandle) {
    struct handleEntry *handleTable = malloc(256 * sizeof(struct handleEntry));
    handleTable[0] = firstHandle;
}
