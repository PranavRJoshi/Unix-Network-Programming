CC=gcc
CFLAGS=-pedantic -ansi -std=c89
MEM_LEAK=-fsanitize=address

CLONE=clone_file.c

all: open_file

open_file: open_file.c
	$(CC) $(CFLAGS) $(MEM_LEAK) -o $@ $<

clean:
	rm open_file $(CLONE)

