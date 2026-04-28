
HandleTable createHandleTable(int size);
int addHandleEntry(char *handleName, int socketNum, HandleTable *table);
int expandHandleTable(HandleTable *table);
int removeHandleEntry(HandleEntry handle, HandleTable *table);
bool lookUpHandle (char * handleName, HandleTable *table);
bool lookUpSocket (int socketNum, HandleTable *table);

