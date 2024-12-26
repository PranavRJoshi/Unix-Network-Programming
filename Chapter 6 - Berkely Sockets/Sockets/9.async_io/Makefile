CC=gcc
CFLAGS=-O -Wall -W -pedantic -ansi -std=c99

EXEC=async_io
OBJS=async_io.o

all: async_io

async_io: async_io.o
	$(CC) $(CFLAGS) -o $@ $<

async_io.o: async_io.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)
