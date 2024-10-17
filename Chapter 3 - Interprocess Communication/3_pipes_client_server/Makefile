CC=gcc
CFLAGS=-Wall -W -pedantic -ansi -std=c89

EXEC=main
OBJS=main.o client.o server.o err_routine.o

all: main

main: main.o client.o server.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c client.h server.h err_routine.h
	$(CC) $(CFLAGS) -c $<

client.o: client.c client.h err_routine.h
	$(CC) $(CFLAGS) -c $<

server.o: server.c server.h err_routine.h
	$(CC) $(CFLAGS) -c $<

err_routine.o: err_routine.c err_routine.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)

