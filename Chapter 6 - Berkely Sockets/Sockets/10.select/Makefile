CC=gcc
CFLAGS=-O -Wall -W -pedantic -ansi -std=c99 -fsanitize=address

EXEC=select stdin_stdout_select
OBJS=select.o stdin_stdout_select.o

all: $(EXEC)

select: select.o
	$(CC) $(CFLAGS) -o $@ $<

select.o: select.c
	$(CC) $(CFLAGS) -c $<

stdin_stdout_select: stdin_stdout_select.o
	$(CC) $(CFLAGS) -o $@ $<

stdin_stdout_select.o: stdin_stdout_select.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)
