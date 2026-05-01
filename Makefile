# Makefile for CPE464 tcp test code
# written by Hugh Smith - April 2019
#edited for chat

CC= gcc
CFLAGS= -g -Wall -std=gnu99
LIBS = 

COMMON_OBJS = networks.o gethostbyname.o pollLib.o safeUtil.o sendRecvPDU.o
CLIENT_OBJS = $(COMMON_OBJS) handleTable.o
SERVER_OBJS = $(COMMON_OBJS) handleTable.o

all:   cclient server

cclient: cclientPollCopy.c $(OBJS)
	$(CC) $(CFLAGS) -o cclient cclientPollCopy.c  $(OBJS) $(LIBS)

server: serverPollCopy.c $(OBJS)
	$(CC) $(CFLAGS) -o server serverPollCopy.c $(OBJS) $(LIBS)
	
.c.o:
	gcc -c $(CFLAGS) $< -o $@

cleano:
	rm -f *.o

clean:
	rm -f server cclient *.o




