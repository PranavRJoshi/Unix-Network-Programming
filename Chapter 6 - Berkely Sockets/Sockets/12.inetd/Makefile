CC=gcc
MEM_LEAK=-fsanitize=address
CFLAGS=-O -Wall -W -pedantic -ansi -std=c99 $(MEM_LEAK)

EXEC=inetd str_echo str_dis dg_echo dg_dis
OBJS=inetd.o str_echo.o str_dis.o dg_echo.o dg_dis.o readline.o write_log.o writen.o

all: $(EXEC)

inetd: inetd.o write_log.o
	$(CC) $(CFLAGS) -o $@ $^

inetd.o: inetd.c utils.h
	$(CC) $(CFLAGS) -c $<

str_echo: str_echo.o readline.o writen.o
	$(CC) $(CFLAGS) -o $@ $^

str_echo.o: str_echo.c utils.h
	$(CC) $(CFLAGS) -c $<

str_dis: str_dis.o writen.o readline.o 
	$(CC) $(CFLAGS) -o $@ $^

str_dis.o: str_dis.c utils.h
	$(CC) $(CFLAGS) -c $<

dg_echo: dg_echo.o
	$(CC) $(CFLAGS) -o $@ $<

dg_echo.o: dg_echo.c utils.h
	$(CC) $(CFLAGS) -c $<

dg_dis: dg_dis.o
	$(CC) $(CFLAGS) -o $@ $<

dg_dis.o: dg_dis.c utils.h
	$(CC) $(CFLAGS) -c $<

readline.o: readline.c utils.h
	$(CC) $(CFLAGS) -c $<

write_log.o: write_log.c utils.h
	$(CC) $(CFLAGS) -c $<

writen.o: writen.c utils.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(EXEC) $(OBJS)


