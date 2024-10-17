CC=gcc
CFLAGS=-Wall -W -pedantic -ansi -std=c89

EXEC=fifo_client fifo_server
OBJS=fifo_client.o fifo_server.o fifo.o err_routine.o

# main is separate from the fifo_{client/server} program.
# main has it's own main entry point whereas fifo_client has its own main entry point
# as well as fifo_server has its own main entry point
#
# The program illustrates the extended use case of pipes between process which do not
# have common parent process (using fork) and fifo_{client/server} shows different 
# processes being able to use named pipes (FIFOs).

all: fifo_client fifo_server

main: main.o fifo.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c fifo.h err_routine.h
	$(CC) $(CFLAGS) -c $<

fifo_client: fifo_client.o fifo.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

fifo_server: fifo_server.o fifo.o err_routine.o
	$(CC) $(CFLAGS) -o $@ $^

fifo_client.o: fifo_client.c fifo.h err_routine.h
	$(CC) $(CFLAGS) -c $<

fifo_server.o: fifo_server.c fifo.h err_routine.h
	$(CC) $(CFLAGS) -c $<

fifo.o: fifo.c fifo.h err_routine.h
	$(CC) $(CFLAGS) -c $<

err_routine.o: err_routine.c err_routine.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)

cleanmain:
	rm main main.o fifo.o err_routine.o
