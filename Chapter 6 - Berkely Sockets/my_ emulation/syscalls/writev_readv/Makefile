CC=gcc
OPTIMIZE=-O
WARNINGS=-Wall -W
STANDARD=c99
CFLAGS=$(OPTIMIZE) $(WARNINGS) -pedantic -ansi -std=$(STANDARD) -fsanitize=address

EXEC=readv writev
OBJS=readv.o writev.o

all: readv writev

readv: readv.o
	$(CC) $(CFLAGS) -o $@ $<

readv.o: readv.c
	$(CC) $(CFLAGS) -c $<

writev: writev.o
	$(CC) $(CFLAGS) -o $@ $<

writev.o: writev.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)
