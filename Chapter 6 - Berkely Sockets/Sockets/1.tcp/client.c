#include "common.h"
#include "inet.h"
#include <strings.h>    /* for bzero() */

int main (int argc, char **argv) {
  int                   sockfd;
  struct sockaddr_in    serv_addr;

  pname = argv[0];

  /*
   * Fill in the structure "serv_addr" with the address of the server that we want to connect with.
  */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
  serv_addr.sin_port        = htons(SERV_TCP_PORT);

  /*
   * Open a TCP socket (an Internet stream socket).
  */
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("client: can't open stream socket");         /* err_sys used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Connect to the server.
  */
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("client: can't connect to the server.");     /* err_sys used here */
    exit(EXIT_FAILURE);
  }

  str_cli(stdin, sockfd);       /* do it all */

  close(sockfd);

  exit(EXIT_SUCCESS);
}
