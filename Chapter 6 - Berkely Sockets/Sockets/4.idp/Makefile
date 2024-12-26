CC=gcc
CFLAGS=-O -W -Wall -pedantic -ansi -std=c99 

all: server client

server: server.o dg_echo.o readline.o writen.o
	$(CC) $(CFLAGS) -o $@ $^

server.o: server.c xns.h common.h
	$(CC) $(CFLAGS) -c $<

client: client.o dg_cli.o readline.o writen.o
	$(CC) $(CFLAGS) -o $@ $^

client.o: client.c common.h xns.h
	$(CC) $(CFLAGS) -c $<

dg_cli.o: dg_cli.c common.h 
	$(CC) $(CFLAGS) -c $<

dg_echo.o: dg_echo.c common.h 
	$(CC) $(CFLAGS) -c $<

readline.o: readline.c common.h
	$(CC) $(CFLAGS) -c $<

writen.o: writen.c common.h
	$(CC) $(CFLAGS) -c $<

clean: 
	rm server server.o dg_echo.o readline.o writen.o client client.o dg_cli.o
