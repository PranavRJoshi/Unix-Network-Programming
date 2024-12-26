#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>   /* Make gcc (or clang :( in my case) ensure that sockaddr is a thing */

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
 * dg_cli:  Read the contents of the FILE *fp, write each line to the datagram socket and write it to the standard output.
 *          Return to caller when an EOF is encountered on the input file.
*/
void dg_cli (FILE *fp, int sockfd, struct sockaddr *pserv_addr, int servlen);

/*
 * dg_echo: Read a datagram from a connectionless socket and write it back to the sender.
 *          We never return, as we never know when a datagram client is done.
*/
void dg_echo (int sockfd, struct sockaddr *pcli_addr, int maxclilen);

#endif
