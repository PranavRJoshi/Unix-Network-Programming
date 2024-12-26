#include "xns.h"
#include "common.h"

int main (int argc, char **argv) {

  int                 sockfd;
  struct sockaddr_ns  cli_addr, serv_addr;

  pname = argv[0];

  /*
   * Fill in the structure "serv_addr" with the XNS addresses of the server that we want to send to.
  */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sns_family  = AF_NS;
  serv_addr.sns_addr    = ns_addr(IDP_SERV_ADDR);     /* Won't even compile as the ns_addr() definition does not exist */
                                                      /* stores net-ID, host-ID, and port */
  /* 
   * I can't find the implementation for `ns_addr()` on the web, but even if I could,
   * it really doesn't change the fact that a socket supporting XNS protocol family can't be made... on my machine
  */
  
  /*
   * Open an IDP socket (an XNS datagram socket).
  */
  if ( (sockfd = socket(AF_NS, SOCK_DGRAM, 0)) < 0) {
    perror("client: can't open stream socket.");      /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Bind any local address for us.
  */
  bzero((char *) &cli_addr, sizeof(cli_addr));    /* zero out */
  cli_addr.sns_family = AF_NS;
  if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
    perror("client: can't bind local address.");      /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  dg_cli(stdin, sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

  close(sockfd);
  
  exit(EXIT_FAILURE);
}
