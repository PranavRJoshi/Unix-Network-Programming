#include "unix.h"
#include "common.h"

int main (int argc, char **argv) {

  int                     sockfd, servlen;
  struct sockaddr_un      serv_addr, cli_addr;

  pname = argv[0];

  /*
   * Open a socket (a UNIX domain datagram socket).
  */
  if ( (sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ) {
    perror("server: can't open datagram socket");     /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Bind our local address so that the client can send to us.
  */
  unlink(UNIXDG_PATH);        /* in case it was left from last time */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, UNIXDG_PATH);
  #ifdef SUN_LEN 
  servlen = SUN_LEN(&serv_addr);
  #else   /* !SUN_LEN, used the one described in text */
  servlen = sizeof(serv_addr.sun_family) + strlen(serv_addr.sun_path);
  #endif

  if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("server: can't bind local address");     /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  dg_echo(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr));

  /* NOT REACHED */

  exit(EXIT_SUCCESS);
}
