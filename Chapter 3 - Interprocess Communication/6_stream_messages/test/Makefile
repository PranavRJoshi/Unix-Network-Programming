CC=gcc
CFLAGS=-O -Wall -W -pedantic -ansi -std=c89

all: test

test: test.o
	$(CC) $(CFLAGS) -o $@ $<

test.o: test.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm test test.o
