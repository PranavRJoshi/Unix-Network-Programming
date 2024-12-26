#include "common.h"
#include <stdio.h>

#define MAXLINE   512

void str_echo (int sockfd) {
  int   n;
  char  line[MAXLINE];

  for (;;) {
    n = readline(sockfd, line, MAXLINE);

    if (n == 0) {
      return;
    } else if (n < 0) {
      perror("str_echo: readline error.");
      exit(EXIT_FAILURE);
    }

    if (writen(sockfd, line, n) != n) {
      perror("str_echo: writen error.");
      exit(EXIT_FAILURE);
    }
  }
}
