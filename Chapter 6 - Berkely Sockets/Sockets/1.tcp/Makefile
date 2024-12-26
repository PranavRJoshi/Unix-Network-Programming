CC=gcc
CFLAGS=-Wall -W -pedantic -ansi -std=c99

all: client server

client: client.o str_cli.o readline.o writen.o
	$(CC) $(CFLAGS) -o $@ $^

server: server.o str_echo.o	readline.o writen.o
	$(CC) $(CFLAGS) -o $@ $^

client.o: client.c common.h inet.h
	$(CC) $(CFLAGS) -c $<

server.o: server.c common.h inet.h
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
	rm client server client.o server.o str_cli.o str_echo.o readline.o writen.o
