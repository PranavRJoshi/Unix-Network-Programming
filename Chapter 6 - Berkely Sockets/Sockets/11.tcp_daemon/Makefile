CC=gcc
CFLAGS=-pedantic -ansi -std=c99
MEM_LEAK=-fsanitize=address

EXEC=tcp_daemon
OBJS=tcp_daemon.o str_echo.o readline.o write_log.o writen.o

all: $(EXEC)

tcp_daemon: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

tcp_daemon.o: tcp_daemon.c utils.h
	$(CC) $(CFLAGS) -c $<

str_echo.o: str_echo.c utils.h
	$(CC) $(CFLAGS) -c $<

readline.o: readline.c utils.h
	$(CC) $(CFLAGS) -c $<

write_log.o: write_log.c utils.h
	$(CC) $(CFLAGS) -c $<

writen.o: writen.c utils.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)

