CC=gcc
CFLAGS=-O -Wall -W -pedantic -ansi -std=c89

EXEC=main
OBJS=main.o semaphore.o err_routine.o

all: main

main: main.o semaphore.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

main.o:	main.c semaphore.h err_routine.h
	$(CC) $(CFLAGS) -c $<

semaphore.o: semaphore.c semaphore.h err_routine.h
	$(CC) $(CFLAGS) -c $<

err_routine.o: err_routine.c err_routine.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)

