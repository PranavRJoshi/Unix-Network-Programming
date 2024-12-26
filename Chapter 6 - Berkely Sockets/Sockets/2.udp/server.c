#include "common.h"
#include "inet.h"
#include <strings.h>    /* for bzero() */

int main (int argc, char **argv) {
  
  int                   sockfd;
  struct sockaddr_in    cli_addr, serv_addr;

  pname = argv[0];      /* store the process name, i.e. `./server` to pname */

  /*
   * Open a UDP socket (an Internet Datagram Socket)
  */
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("server: can't open datagram socket.");    /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Bind our local address so that the client can send to us.
  */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family        = AF_INET;
  serv_addr.sin_addr.s_addr   = htonl(INADDR_ANY);
  serv_addr.sin_port          = htons(SERV_UDP_PORT);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("server: can't bind local address.");    /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  dg_echo(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr));

  /* NOT REACHED */

  exit(EXIT_SUCCESS);
}
