CC=gcc
CFLAGS=-pedantic -ansi -std=c89

all: check_env 

check_env: check_env.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm check_env 


