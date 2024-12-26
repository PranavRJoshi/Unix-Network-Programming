CC=gcc
OPTIMIZE=-O
WARNINGS=-Wall -W
STANDARD=c99
SANITIZE=-fsanitize=address
CFLAGS=$(OPTIMIZE) $(WARNINGS) -pedantic -ansi -std=$(STANDARD) $(SANITIZE)

EXEC=get_set_sockopt sock_fcntl	sock_ioctl
OBJS=get_set_sockopt.o sock_fcntl.o sock_ioctl.o

all: $(EXEC)

get_set_sockopt: get_set_sockopt.o
	$(CC) $(CFLAGS) -o $@ $<

get_set_sockopt.o: get_set_sockopt.c
	$(CC) $(CFLAGS) -c $<

sock_fcntl: sock_fcntl.o
	$(CC) $(CFLAGS) -o $@ $<

sock_fcntl.o: sock_fcntl.c
	$(CC) $(CFLAGS) -c $<

sock_ioctl: sock_ioctl.o
	$(CC) $(CFLAGS) -o $@ $<

sock_ioctl.o: sock_ioctl.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)
