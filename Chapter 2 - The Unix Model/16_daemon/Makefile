CC=gcc
CFLAGS=-pedantic -ansi -std=c89
MEM_LEAK=-fsanitize=address

all: daemon 

daemon: daemon.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm daemon

