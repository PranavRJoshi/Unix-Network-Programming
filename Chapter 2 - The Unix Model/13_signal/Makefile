CC=gcc
CFLAGS=-pedantic -ansi -std=c89
MEM_LEAK=-fsanitize=address

all: signal signal2

signal: signal.c
	$(CC) $(CFLAGS) -o $@ $<

signal2: signal2.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm signal signal2

