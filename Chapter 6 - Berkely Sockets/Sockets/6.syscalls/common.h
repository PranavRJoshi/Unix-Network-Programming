#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

struct header_format {
  int     type;               /* dummy. header type for no reason :p */
  int     version;            /* version */
  char    protocol_name[10];  /* name of the dummy protocol: UNP */
};

#define VERSION_ERROR     "[ERROR] Different Version"
#define NO_VERSION_ERROR  "[SUCCESS] Same Version"

#define PROTO_NAME_ERR    "[ERROR] Invalid Protocol"
#define NO_PROTO_NAME_ERR "[SUCCESS] Valid Protocol"

/* test, remove later */

typedef unsigned int my_size_t;

struct my_iovec {
  void            *address;
  my_size_t       buff_length;
};

my_size_t my_readv (int fd, const struct my_iovec *my_iov, int my_iov_cnt);

my_size_t my_writev (int fd, const struct my_iovec *my_iov, int my_iov_cnt);

/*
 * readn: Read 'n' bytes from a descriptor.
 *        Use in place of read() when fd is a stream socket.
*/
int readn (register int fd, register char *ptr, register int nbytes);

/*
 * writen:  Write 'n' bytes to a descriptor.
 *          Use in place of write() when fd is a stream socket.
*/
int writen (register int fd, register char *ptr, register int nbytes);

/*
 * readline:  Read a line from a descriptor. Read the line one byte at a time, looking for the newline.
 *            We store the newline in the buffer, then follow it with a null character (the same as fgets(3)).
 *            We return the number of characters up to, but not including, the null character (same as strlen(3)).
*/
int readline (register int fd, register char *ptr, register int maxline);

/*
 * str_cli: Read the contents of the FILE *fp, write each line to the stream socket (to the server process),
 *          then read a line back from the socket and write it back to the standard output.
 *
 *          Return to caller when an EOF is encountered on the input file.
*/
void str_cli (register FILE *fp, register int sockfd);

/*
 * str_echo:  Read a stream socket one line at a time, and write each line back to the sender.
 *            Return when the connection is terminated.
*/
void str_echo (int sockfd);

#endif
