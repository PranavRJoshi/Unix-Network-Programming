CC=gcc
CFLAGS=-pedantic -ansi -std=c89
MEM_LEAK=-fsanitize=address

all: fork 

fork: fork.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm fork 

