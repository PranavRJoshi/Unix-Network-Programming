#include "utils.h"

#define MAXLINE   512
#define QUIT_CMD  "tcpquit"

int main (int argc, char **argv) {
  
  if (argc != 1) {
    /* usage: ./str_echo */
    exit(EXIT_FAILURE);
  }

  int   n;
  char  line[MAXLINE];

  for (;;) {
    n = readline(0, line, MAXLINE);

    if (n == 0) {
      return 0;
    } else if (n < 0) {
      exit(EXIT_FAILURE);
    }
    
    if (strncmp(line, QUIT_CMD, 8) == 0) {
      break;
    }

    if (writen(1, line, n) != n) {
      exit(EXIT_FAILURE);
    }
  }

  close(0);
  close(1);
  close(2);
  exit(EXIT_SUCCESS);
}

