CC=gcc
CFLAGS=-O -Wall -W -pedantic -ansi -std=c89

all: msg

msg: msg.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

msg.o: msg.c err_routine.h
	$(CC) $(CFLAGS) -c $<

err_routine.o: err_routine.c err_routine.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm msg msg.o err_routine.o
