CC=gcc
CFLAGS=-pedantic -ansi -std=c89
MEM_LEAK=-fsanitize=address

all: exec 

exec: exec.c dummy
	$(CC) $(CFLAGS) -o $@ $<

dummy: dummy.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm exec dummy

