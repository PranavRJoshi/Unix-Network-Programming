CC=gcc
CFLAGS=-Wall -W -pedantic -ansi -std=c89

EXEC=popenclose
OBJS=popenclose.o err_routine.o

all: popenclose

popenclose: popenclose.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

popenclose.o: popenclose.c err_routine.h
	$(CC) $(CFLAGS) -c $<

err_routine.o: err_routine.c err_routine.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)


