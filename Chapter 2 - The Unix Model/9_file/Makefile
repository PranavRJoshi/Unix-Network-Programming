CC=gcc
CFLAGS=-pedantic -ansi -std=c89

all: file typeof_file

file: file.c
	$(CC) $(CFLAGS) -o $@ $<

typeof_file: typeof_file.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm file typeof_file

