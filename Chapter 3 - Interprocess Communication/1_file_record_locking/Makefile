CC=gcc
CFLAGS=-Wall -W -pedantic -ansi -std=c89
ADDR_SAN=-fsanitize=address

EXEC = file_record
OBJS = file_record.o lock.o err_routine.o

all: file_record

file_record: file_record.o lock.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

file_record.o: file_record.c lock.h
	$(CC) $(CFLAGS) -c $<

lock.o: lock.c lock.h
	$(CC) $(CFLAGS) -c $<

err_routine.o: err_routine.c err_routine.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)
