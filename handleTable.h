#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H

#include <stdbool.h>
#include <stdint.h>

HandleTable createHandleTable(int size);
int addHandleEntry(char *handleName, int socketNum, HandleTable *table);
int expandHandleTable(HandleTable *table);
int removeHandleEntry(HandleEntry handle, HandleTable *table);
bool lookUpHandle (char * handleName, HandleTable *table);
bool lookUpSocket (int socketNum, HandleTable *table);

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

#endif