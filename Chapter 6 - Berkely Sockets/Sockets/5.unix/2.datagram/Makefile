CC=gcc
CFLAGS=-Wall -W -pedantic -ansi -std=c99

UNIXDG_PATH=s.unixdg

all: client server

client: client.o dg_cli.o readline.o writen.o
	$(CC) $(CFLAGS) -o $@ $^

server: server.o dg_echo.o	readline.o writen.o
	$(CC) $(CFLAGS) -o $@ $^

client.o: client.c common.h unix.h
	$(CC) $(CFLAGS) -c $<

server.o: server.c common.h unix.h
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
	rm client server client.o server.o dg_cli.o dg_echo.o readline.o writen.o	$(UNIXDG_PATH)
