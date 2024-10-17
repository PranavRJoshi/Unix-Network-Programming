CC=gcc
CFLAGS=-Wall -W -pedantic -ansi -std=c89

EXEC=main
OBJS=main.o msg.o ipc_client.o ipc_server.o err_routine.o

all: main

main: main.o msg.o ipc_client.o ipc_server.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c err_routine.h ipc.h
	$(CC) $(CFLAGS) -c $<

msg.o: msg.c msg.h err_routine.h
	$(CC) $(CFLAGS) -c $<

ipc_client.o: ipc_client.c ipc.h msg.h err_routine.h
	$(CC) $(CFLAGS) -c $<

ipc_server.o: ipc_server.c ipc.h msg.h err_routine.h
	$(CC) $(CFLAGS) -c $<

err_routine.o: err_routine.c err_routine.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)

