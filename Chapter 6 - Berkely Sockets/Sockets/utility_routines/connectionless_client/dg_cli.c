#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#define MAXLINE   512

/*
 * dg_cli:  Read the contents of the FILE *fp, write each line to the datagram socket and write it to the standard output.
 *          Return to caller when an EOF is encountered on the input file.
*/

/*
 * C89 function definition format:
 *
 *  dg_cli (fp, sockfd, pserv_addr, servlen)
 *  FILE              *fp;
 *  int               sockfd;
 *  struct sockaddr   *pserv_addr;    // ptr to appropriate sockaddr_XX structure
 *  int               servlen; {      // actual sizeof(*pserv_addr)
 *    ...   // function with no return type, defaults to int
 *  }
*/

void dg_cli (FILE *fp, int sockfd, struct sockaddr *pserv_addr, int servlen) {
  int   n;
  char  sendline[MAXLINE], recvline[MAXLINE + 1];

  while (fgets(sendline, MAXLINE, fp) != NULL) {
    n = strlen(sendline);

    if (sendto(sockfd, sendline, n, 0, pserv_addr, servlen) != n) {
      perror("dg_cli: sendto error on socket");     /* err_dump used */
      exit(EXIT_FAILURE);
    }

    /*
     * Now, read a message from the socket and write it to our standard output.
    */
    /* text uses `(int *) 0` instead of `(socklen_t *) 0`. Did it to make the compiler happy :) */
    n = recvfrom(sockfd, recvline, MAXLINE, 0, (struct sockaddr *) 0, (socklen_t *) 0);

    if (n < 0) {
      perror("dg_cli: recvfrom error.");     /* err_dump used */
      exit(EXIT_FAILURE);
    }

    recvline[n] = 0;    /* null termination */
    fputs(recvline, stdout);
  }

  if (ferror(fp)) {
    perror("dg_cli: error reading file.");   /* err_dump used */
    exit(EXIT_FAILURE);
  }
}
