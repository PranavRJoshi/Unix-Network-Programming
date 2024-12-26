#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>    /* for perror */
#include <stdlib.h>   /* for exit */

/*
 * dg_echo: Read a datagram from a connectionless socket and write it back to the sender.
 *          We never return, as we never know when a datagram client is done.
*/

#define MAXMESG   2048

/*
 * C89 function defintion format:
 *
 *  dg_echo (sockfd, pcli_addr, maxclilen)
 *  int                 sockfd;
 *  struct sockaddr     *pcli_addr;     // ptr to appropriate sockaddr_XX structure
 *  int                 maxclilen; {    // sizeof(*pcli_addr)
 *    ...     // function with no return type, defaults to int
 *  }
*/

void dg_echo (int sockfd, struct sockaddr *pcli_addr, int maxclilen) {
  int   n, clilen;
  char  mesg[MAXMESG];

  for (;;) {
    clilen = maxclilen;
    /* Warning below, but we don't really care as clilen is a value-result argument, we are not sending value, rather the call assigns the value (size of the network address structure) to the variable clilen */
    n = recvfrom(sockfd, mesg, MAXMESG, 0, pcli_addr, &clilen);
    if (n < 0) {
      perror("dg_echo: recvfrom error.");   /* err_dump used here. */
      exit(EXIT_FAILURE);
    }

    if (sendto(sockfd, mesg, n, 0, pcli_addr, clilen) != n) {
      perror("dg_echo: sendto error.");     /* err_dump used here. */
      exit(EXIT_FAILURE);
    }
  }
}
