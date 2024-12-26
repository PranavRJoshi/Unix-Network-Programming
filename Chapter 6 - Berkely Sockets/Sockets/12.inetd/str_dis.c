#include "utils.h"

#define MAXLINE   512
#define DIS_CMD  "tcpdiscard"

int main (int argc, char **argv) {
  
  if (argc != 1) {
    /* usage: ./str_discard <sockfd> */
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

    if (strncmp(line, DIS_CMD, 10) == 0) {
      // write(1, "closing\n", 9);
      break;
    }
  }

  close(0);
  close(1);
  close(2);
  exit(EXIT_SUCCESS);
}
