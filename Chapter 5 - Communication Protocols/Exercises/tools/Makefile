CC=gcc
CFLAGS=-O -Wall -W -pedantic -ansi -std=c2x

EXEC=iptohex sockaddr
OBJS=iptohex.o sockaddr.o

TEST=test

all: iptohex sockaddr

iptohex: iptohex.o
	$(CC) $(CFLAGS) -o $@ $<

sockaddr: sockaddr.o
	$(CC) $(CFLAGS) -o $@ $<

sockaddr.o: sockaddr.c
	$(CC) $(CFLAGS) -c $<

test: iptohex.c
	$(CC) $(CFLAGS) -c $< -D_TEST_FUNCTION 
	$(CC) $(CFLAGS) -o test iptohex.o

iptohex.o: iptohex.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS) $(TEST)
