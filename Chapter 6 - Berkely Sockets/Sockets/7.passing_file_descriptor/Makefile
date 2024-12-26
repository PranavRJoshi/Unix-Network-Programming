CC=gcc
OPTIMIZE=-O
WARNINGS=-Wall -W
STANDARD=c99
CFLAGS=$(OPTIMIZE) $(WARNINGS) -pedantic -ansi -std=$(STANDARD)

EXEC=mycat openfile
OBJS=mycat.o my_open.o s_pipe.o send_recv_file.o err_routine.o openfile.o

all: mycat openfile

mycat: mycat.o my_open.o s_pipe.o send_recv_file.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

mycat.o: mycat.c common.h err_routine.h
	$(CC) $(CFLAGS) -c $<

my_open.o: my_open.c common.h err_routine.h
	$(CC) $(CFLAGS) -c $<

openfile: openfile.o send_recv_file.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

openfile.o: openfile.c common.h err_routine.h
	$(CC) $(CFLAGS) -c $<

send_recv_file.o: send_recv_file.c common.h
	$(CC) $(CFLAGS) -c $<

err_routine.o: err_routine.c err_routine.h systype.h
	$(CC) $(CFLAGS) -c $<

s_pipe.o: s_pipe.c common.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)
