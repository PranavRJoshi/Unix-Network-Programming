CC=gcc
CFLAGS=-O -Wall -W -pedantic -ansi -std=c99

EXEC=sockaddr bind
OBJS=sockaddr.o bind.o
TEST=client server
TCP_TEST=tcp_server tcp_client
TCP=accept.c

all: sockaddr bind accept

test: tcp

sockaddr: sockaddr.o
	$(CC) $(CFLAGS) -o $@ $<

sockaddr.o: sockaddr.c
	$(CC) $(CFLAGS) -c $<

bind: bind.o
	$(CC) $(CFLAGS) -o $@ $<

bind.o: bind.c
	$(CC) $(CFLAGS) -c $<

cliserv: $(TCP_TEST)

tcp_server: $(TCP)
	$(CC) $(CFLAGS) -D_TCP_SERVER -o $@ $(TCP)

tcp_client: $(TCP)
	$(CC) $(CFLAGS) -o $@ $(TCP)

accept.o: accept.c
	$(CC) $(CFLAGS) -c $< 

# @ at the beginning indicates that the command will be executed but will not be displayed in the standard output.
tcp: sockaddr.c bind.c
	@ echo "Building sockaddr.c and bind.c with _CHECK_TCP macro enabled"
	@ $(CC) $(CFLAGS) -c sockaddr.c -D_CHECK_TCP
	@ $(CC) $(CFLAGS) -c bind.c -D_CHECK_TCP
	@ $(CC) $(CFLAGS) -o server bind.o
	@ $(CC) $(CFLAGS) -o client sockaddr.o
	@ rm sockaddr.o bind.o
	@ echo "Created" $(TEST_TCP) "executables and removed the" $(OBJS) "files"

clean:
	rm $(EXEC) $(OBJS) $(TEST)

clean_test:
	rm $(TEST)

clean_tcp:
	rm $(TCP_TEST)

