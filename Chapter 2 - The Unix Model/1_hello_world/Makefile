CC=gcc
CFLAGS=-pedantic -ansi -std=c89 

all: hello_world

hello_world: hello_world.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm hello_world
