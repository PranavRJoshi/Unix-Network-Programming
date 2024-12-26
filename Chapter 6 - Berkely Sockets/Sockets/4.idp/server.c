#include "xns.h"
#include "common.h"

extern int errno;

int main (int argc, char **argv) {

  int                 sockfd, newsockfd, clilen;
  struct sockaddr_ns  cli_addr, serv_addr;

  pname = argv[0];

  /*
   * Open an IDP socket (an XNS Datagram socket).
  */
  if ( (sockfd = socket(AF_NS, SOCK_DGRAM, 0)) < 0) {
    perror("server: can't open stream socket");       /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Bind our local address so that the client can send to us.
  */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sns_family      = AF_NS;
  serv_addr.sns_addr.x_port = htons(SERV_IDP_PORT);
  if ( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) {
    perror("server: can't bind local address.");      /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  dg_echo(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr));

  /* NOT REACHED */

  exit(EXIT_SUCCESS);
}
