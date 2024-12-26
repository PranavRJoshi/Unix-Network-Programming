#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXMESG   2048

void dg_echo (int sockfd, struct sockaddr *pcli_addr, int maxclilen) {
  int   n, clilen;
  char  mesg[MAXMESG];

  for (;;) {
    clilen = maxclilen;
    n = recvfrom(sockfd, mesg, MAXMESG, 0, pcli_addr, &clilen);
    if (n < 0) {
      perror("dg_echo: recvfrom error.");
      exit(EXIT_FAILURE);
    }

    if (sendto(sockfd, mesg, n, 0, pcli_addr, clilen) != n) {
      perror("dg_echo: sendto error.");
      exit(EXIT_FAILURE);
    }
  }
}
