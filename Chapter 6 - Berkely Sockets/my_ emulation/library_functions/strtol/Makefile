CC=gcc 
OPTIMIZE=-O 
WARNINGS=-W -Wall
STANDARD=c99
SEGV=-fsanitize=address
DEBUG=-g

CFLAGS=$(OPTIMIZE) $(WARNINGS) -pedantic -ansi -std=$(STANDARD)

EXEC=strtol
OBJS=strtol.o
DEBUG_DIR=strtol.dSYM/

all: strtol

strtol: strtol.o
	$(CC) $(CFLAGS) -o $@ $<

strtol.o: strtol.c
	$(CC) $(CFLAGS) -c $<

debug: strtol.c
	$(CC) $(CFLAGS) $(DEBUG) $(SEGV) -o $(EXEC) $<

clean:
	rm $(EXEC) $(OBJS)

cleandebug:
	rm -r $(DEBUG_DIR)
	rm $(EXEC)
