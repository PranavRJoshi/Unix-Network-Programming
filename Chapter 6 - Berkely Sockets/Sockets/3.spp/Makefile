CC=gcc
CFLAGS=-O -W -Wall -pedantic -ansi -std=c99 

all: server client

server: server.o str_echo.o readline.o writen.o
	$(CC) $(CFLAGS) -o $@ $^

server.o: server.c xns.h common.h
	$(CC) $(CFLAGS) -c $<

client: client.o str_cli.o readline.o writen.o
	$(CC) $(CFLAGS) -o $@ $^

client.o: client.c common.h xns.h
	$(CC) $(CFLAGS) -c $<

str_cli.o: str_cli.c common.h 
	$(CC) $(CFLAGS) -c $<

str_echo.o: str_echo.c common.h 
	$(CC) $(CFLAGS) -c $<

readline.o: readline.c common.h
	$(CC) $(CFLAGS) -c $<

writen.o: writen.c common.h
	$(CC) $(CFLAGS) -c $<

clean: 
	rm server server.o str_echo.o readline.o writen.o client client.o str_cli.o
