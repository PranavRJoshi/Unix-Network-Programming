CC=gcc
CFLAGS=-pedantic -ansi -std=c89

all: environ environ2

environ: environ.c
	$(CC) $(CFLAGS) -o $@ $<

environ2: environ2.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm environ environ2

