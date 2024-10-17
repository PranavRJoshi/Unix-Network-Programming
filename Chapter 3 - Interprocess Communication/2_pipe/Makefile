CC=gcc
CFLAGS=-Wall -W -pedantic -ansi -std=c89

EXEC=pipe
OBJS=pipe.o err_routine.o

all: pipe

pipe: pipe.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

pipe.o: pipe.c err_routine.h
	$(CC) $(CFLAGS) -c $<

err_routine.o: err_routine.c err_routine.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)
