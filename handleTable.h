#ifndef HANDLETABLE_H
#define HANDLETABLE_H

#include <stdbool.h>
#include <stdint.h>

HandleTable createHandleTable(int size);
int addHandleEntry(char *handleName, int socketNum);
int expandHandleTable(void);
bool lookUpHandle(char *handleName);
bool lookUpSocket(int socketNum);
int removeHandleBySocket(int socketNum);
int getSocketByHandle(char *handleName);
char *getHandleByIndex(int index);
int getSocketByIndex(int index);
int getHandleCount(void);

HandleTable globalTable;

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