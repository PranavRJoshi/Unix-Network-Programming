#include "../common.h"
#include <stdio.h>        /* Used for FILE structure and its methods like fopen, and such... and also for `perror`... */
#include <string.h>       /* for strlen */
#include <stdlib.h>       /* for exit */

#define MAXLINE   512

/*
 * str_cli: Read the contents of the FILE *fp, write each line to the stream socket (to the server process),
 *          then read a line back from the socket and write it back to the standard output.
 *
 *          Return to caller when an EOF is encountered on the input file.
*/

/*
 * C89 function definition format:
 *
 *  str_cli (fp, sockfd)
 *  register FILE *fp;
 *  register int  sockfd; {
 *    ...   // no function return type is implicitly understood as function with return type of int.
 *  }
*/

void str_cli (register FILE *fp, register int sockfd) {
  int n;
  char sendline[MAXLINE], recvline[MAXLINE + 1];

  while (fgets(sendline, MAXLINE, fp) != NULL) {    /* read from fp and store in sendline, read at most MAXLINE - 1. Returns NULL if error has occurred, else returns the pointer to the string. */
    n = strlen(sendline);   /* fgets appends a null character at the end of the string. check fgets(3) */

    if (writen(sockfd, sendline, n) != n) {     /* Write contents of sendline to sockfd, n bytes */
      perror("str_cli: writen error on socket.");   /* err_sys is used here */
      exit(EXIT_FAILURE);
    }

    /*
     * Now, read a line from the socket and write it to our standard output.
    */

    n = readline(sockfd, recvline, MAXLINE);  /* Read from sockfd, and store in recvline. Retains newline like fgets--and stops reading, and returns the length of string excluding the null character. */

    if (n < 0) {
      perror("str_cli: readline error");
      exit(EXIT_FAILURE);
    }

    recvline[n] = 0;              /* null terminate, readline does it, but still... */
    fputs(recvline, stdout);
  }

  if (ferror(fp)) {
    perror("str_cli: error reading file.");
    exit(EXIT_FAILURE);
  }
}
