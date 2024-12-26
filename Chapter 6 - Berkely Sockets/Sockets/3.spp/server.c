#include "xns.h"
#include "common.h"

extern int errno;

int main (int argc, char **argv) {

  int                 sockfd, newsockfd, clilen;
  struct sockaddr_ns  cli_addr, serv_addr;

  pname = argv[0];

  /*
   * Open a SPP socket (an XNS Stream socket).
  */
  if ( (sockfd = socket(AF_NS, SOCK_STREAM, 0)) < 0) {
    // fprintf(stderr, "[LOG] Errno: %d\n", errno);
    perror("server: can't open stream socket");       /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Bind our local address so that the client can send to us
  */
  bzero((char *) &serv_addr, sizeof(serv_addr));    /* leave network and host ID to be zero, similar to UNADDR_ANY */
  serv_addr.sns_family      = AF_NS;
  serv_addr.sns_addr.x_port = htons(SERV_SPP_PORT);

  if ( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) {
    perror("server: can't bind local address.");      /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  listen(sockfd, 5);

  fprintf(stdout, "[LOG] Created a XNS protocol socket.");

  for (;;) {
    /*
     * Wait for a connection from a client process, then process it without fork()'ing.
     * This is an example of an iterative server.
    */
    clilen      = sizeof(cli_addr);
    newsockfd   = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
      perror("server: accept error");     /* err_dump used here */
      exit(EXIT_FAILURE);
    }

    str_echo(newsockfd);      /* returns when connection is closed. */   

    close(newsockfd);
  }

  exit(EXIT_SUCCESS);
}
