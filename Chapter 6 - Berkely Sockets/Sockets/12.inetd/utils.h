#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LOG_MSG_LEN   128

#define ARR_ELE_CNT(arr)      (sizeof((arr)) / sizeof((arr[0])))

// void  dg_echo     (int sockfd, struct sockaddr *pcli_addr, int maxclilen);
// void  str_discard (int sockfd);
// void  dg_discard  (int sockfd, struct sockaddr *pcli_addr, int maxclilen);

/*
 * utilities for exec'd programs.
*/
int readline      (register int fd, register char *ptr, register int maxlen);
int writen        (register int fd, register char *ptr, register int nbytes);

int   write_log   (int logfd, const char *logmsg);

#endif
