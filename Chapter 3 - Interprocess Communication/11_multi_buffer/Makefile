CC=gcc
CFLAGS=-O -Wall -W -pedantic -ansi -std=c89

EXEC=client server
OBJS=client.o server.o semaphore.o err_routine.o

all: client server

client: client.o err_routine.o semaphore.o
	$(CC) $(CFLAGS) -o $@ $^

server:	server.o err_routine.o semaphore.o
	$(CC) $(CFLAGS) -o $@ $^

client.o: client.c mesg.h shm.h err_routine.h semaphore.h
	$(CC) $(CFLAGS) -c $<

server.o: server.c mesg.h shm.h err_routine.h semaphore.h
	$(CC) $(CFLAGS) -c $<
 
semaphore.o: semaphore.c semaphore.h err_routine.h
	$(CC) $(CFLAGS) -c $<
 
err_routine.o: err_routine.c err_routine.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)
