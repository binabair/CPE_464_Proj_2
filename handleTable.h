#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H

#include <stdbool.h>
#include <stdint.h>

HandleTable createHandleTable(int size);
int addHandleEntry(char *handleName, int socketNum);
int expandHandleTable(HandleTable *table);
int removeHandleEntry(HandleEntry handle);
bool lookUpHandle (char * handleName);
bool lookUpSocket (int socketNum);

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