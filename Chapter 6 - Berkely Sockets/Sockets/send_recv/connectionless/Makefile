CC=gcc
CFLAGS=-O -Wall -W -pedantic -ansi -std=c99

EXEC=client server
OBJS=client.o server.o

all: client server

client: client.o
	$(CC) $(CFLAGS) -o $@ $<

client.o: client.c
	$(CC) $(CFLAGS) -c $<

server: server.o
	$(CC) $(CFLAGS) -o $@ $<

server.o: server.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)
