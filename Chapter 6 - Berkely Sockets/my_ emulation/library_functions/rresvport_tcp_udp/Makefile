CC=gcc
OPTIMIZE=-O
WARNINGS=-Wall -W
SANITIZE_ADDR=-fsanitize=address
STANDARD=c99
CFLAGS=$(OPTIMIZE) $(WARNINGS) -pedantic -ansi -std=$(STANDARD) $(SANITIZE_ADDR)

EXEC=rresvport
OBJS=rresvport.o

all: rresvport

rresvport: rresvport.o
	$(CC) $(CFLAGS) -o $@ $<

rresvport.o: rresvport.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)
