#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>       /* Commonly for read, write, close syscalls */
#include <stdio.h>
#include <stdlib.h>

/*
 * Function declarations which will be used commonly and their definition are available in: 
 *    readn.c 
 *    writen.c 
 *    readline.c 
*/

int readn (register int fd, register char *ptr, register int nbytes);

int writen (register int fd, register char *ptr, register int nbytes);

int readline (register int fd, register char *ptr, register int maxline);


void str_cli (register FILE *fp, register int sockfd);

void str_echo (int sockfd);

void dg_cli (FILE *fp, int sockfd, struct sockaddr *pserv_addr, int servlen);

void dg_echo (int sockfd, struct sockaddr *pcli_addr, int maxclilen);

#endif
