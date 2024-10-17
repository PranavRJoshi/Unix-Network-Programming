CC=gcc
CFLAGS=-Wall -W -pedantic -ansi -std=c89

EXEC=client server
OBJS=client.o server.o err_routine.o mesg.o

all: client server

client: client.o mesg.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

server: server.o mesg.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

client.o: client.c mesg.h msgq.h err_routine.h
	$(CC) $(CFLAGS) -c $<

server.o: server.c mesg.h msgq.h err_routine.h
	$(CC) $(CFLAGS) -c $<

err_routine.o: err_routine.c err_routine.h
	$(CC) $(CFLAGS) -c $<

mesg.o: mesg.c mesg.h err_routine.h msgq.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)

