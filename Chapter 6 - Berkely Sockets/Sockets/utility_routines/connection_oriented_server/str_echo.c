#include "../common.h"
#include <stdio.h>    /* Using for the library function `perror` instead of `err_dump` used in text. Will change later */

/*
 * str_echo:  Read a stream socket one line at a time, and write each line back to the sender.
 *            Return when the connection is terminated.
*/

#define MAXLINE   512

/*
 * C89 Function definition format:
 *
 *  str_echo (sockfd)
 *  int sockfd; {
 *    ...   // function has no return type, so the implicit return type added is that of int.
 *  }
*/

void str_echo (int sockfd) {
  int   n;
  char  line[MAXLINE];

  for (;;) {
    n = readline(sockfd, line, MAXLINE);

    if (n == 0) {           /* readline returns 0 when there is no data to be read from the descriptor. So, connection terminated. */
      return;
    } else if (n < 0) {     /* readline return negative number (-1) when an error occured with read call. `errno` must be filled. */ 
      perror("str_echo: readline error.");    /* err_dump is used here */
      exit(EXIT_FAILURE);
    }

    if (writen(sockfd, line, n) != n) {
      perror("str_echo: writen error.");      /* err_dump is used here */
      exit(EXIT_FAILURE);
    }
  }
}
