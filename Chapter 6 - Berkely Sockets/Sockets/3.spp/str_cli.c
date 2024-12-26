#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE   512

void str_cli (register FILE *fp, register int sockfd) {
  int n;
  char sendline[MAXLINE], recvline[MAXLINE + 1];

  while (fgets(sendline, MAXLINE, fp) != NULL) {
    n = strlen(sendline);

    if (writen(sockfd, sendline, n) != n) {
      perror("str_cli: writen error on socket.");
      exit(EXIT_FAILURE);
    }

    n = readline(sockfd, recvline, MAXLINE);

    if (n < 0) {
      perror("str_cli: readline error");
      exit(EXIT_FAILURE);
    }

    recvline[n] = 0;
    fputs(recvline, stdout);
  }

  if (ferror(fp)) {
    perror("str_cli: error reading file.");
    exit(EXIT_FAILURE);
  }
}
