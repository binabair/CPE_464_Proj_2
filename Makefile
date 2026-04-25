# Makefile for CPE464 tcp test code
# written by Hugh Smith - April 2019

CC= gcc
CFLAGS= -g -Wall -std=gnu99
LIBS = 

OBJS = networks.o gethostbyname.o pollLib.o safeUtil.o sendRecvPDU.o

all:   cclient server

cclient: cclientPoll.c $(OBJS)
	$(CC) $(CFLAGS) -o cclient cclientPoll.c  $(OBJS) $(LIBS)

server: serverPoll.c $(OBJS)
	$(CC) $(CFLAGS) -o server serverPoll.c $(OBJS) $(LIBS)
	
.c.o:
	gcc -c $(CFLAGS) $< -o $@ $(LIBS)

cleano:
	rm -f *.o

clean:
	rm -f server cclient *.o




