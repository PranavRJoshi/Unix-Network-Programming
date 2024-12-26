#include "xns.h"
#include "common.h"

int main (int argc, char **argv) {

  int                 sockfd;
  struct sockaddr_ns  serv_addr;

  pname = argv[0];

  /*
   * Fill in the structure "serv_addr" with the XNS addresses of the server that we want to connect with.
  */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sns_family  = AF_NS;
  serv_addr.sns_addr    = ns_addr(SPP_SERV_ADDR);     /* Won't even compile as the ns_addr() definition does not exist */
                                                      /* stores net-ID, host-ID, and port */
  /* 
   * I can't find the implementation for `ns_addr()` on the web, but even if I could,
   * it really doesn't change the fact that a socket supporting XNS protocol family can't be made... on my machine 
  */
  
  /*
   * Open a SPP socket (an XNS stream socket).
  */
  /*
   * Similar to the error in server.c, the socket won't be created (EAFNOSUPPORT)
  */
  if ( (sockfd = socket(AF_NS, SOCK_STREAM, 0)) < 0) {
    perror("client: can't open stream socket.");      /* err_sys used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Connect to the server.
  */
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("client: can't connect to server.");       /* err_sys used here */
    exit(EXIT_FAILURE);
  }

  str_cli(stdin, sockfd);       /* do it all */

  close(sockfd);
  
  return 0;
}
