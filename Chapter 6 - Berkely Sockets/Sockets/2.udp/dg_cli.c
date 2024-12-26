#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#define MAXLINE   512

void dg_cli (FILE *fp, int sockfd, struct sockaddr *pserv_addr, int servlen) {
  int   n;
  char  sendline[MAXLINE], recvline[MAXLINE + 1];

  while (fgets(sendline, MAXLINE, fp) != NULL) {
    n = strlen(sendline);

    if (sendto(sockfd, sendline, n, 0, pserv_addr, servlen) != n) {
      perror("dg_cli: sendto error on socket");
      exit(EXIT_FAILURE);
    }

    n = recvfrom(sockfd, recvline, MAXLINE, 0, (struct sockaddr *) 0, (socklen_t *) 0);

    if (n < 0) {
      perror("dg_cli: recvfrom error.");
      exit(EXIT_FAILURE);
    }

    recvline[n] = 0;
    fputs(recvline, stdout);
  }

  if (ferror(fp)) {
    perror("dg_cli: error reading file.");
    exit(EXIT_FAILURE);
  }
}
