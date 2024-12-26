#include "unix.h"
#include "common.h"

int main (int argc, char **argv) {

  int                     sockfd, servlen;
  struct sockaddr_un      serv_addr;

  pname = argv[0];

  /*
   * Fill in the structure "serv_addr" with the address of the server that we want to send to.
  */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, UNIXSTR_PATH);
  #if defined (SUN_LEN)   /* struct sockaddr_un contains one more field, sun_len, which is not shown in text. Apple defined this macro */ 
  servlen = SUN_LEN(&serv_addr);
  #else   /* !SUN_LEN */
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
  #endif

  /*
   * Open a socket (an UNIX domain stream socket).
  */
  if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("client: can't open stream socket.");        /* err_sys used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Connect to the server.
  */
  if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: can't connect to server.");         /* err_sys used here */
    exit(EXIT_FAILURE);
  }

  str_cli(stdin, sockfd);       /* do it all */

  close(sockfd);
  exit(EXIT_SUCCESS);
}
