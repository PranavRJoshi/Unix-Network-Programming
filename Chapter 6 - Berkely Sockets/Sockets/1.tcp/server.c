#include "common.h"
#include "inet.h"
#include <strings.h>    /* for bzero() */

int main (int argc, char **argv) {
  
  int                   sockfd, newsockfd, clilen, childpid;
  struct sockaddr_in    cli_addr, serv_addr;

  pname = argv[0];      /* store the process name, i.e. `./server` to pname */

  /*
   * Open a TCP socket (an Internet Stream Socket)
  */
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    perror("server: can't open stream socket.");    /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Bind our local address so that the client can send to us.
  */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family        = AF_INET;
  /*
   * INADDR_ANY tells the system that we will accept a connection on any Internet interface on the system,
   * if the system is multihomed.
  */
  serv_addr.sin_addr.s_addr   = htonl(INADDR_ANY);
  serv_addr.sin_port          = htons(SERV_TCP_PORT);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("server: can't bind local address.");    /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  listen(sockfd, 5);

  for (;;) {
    /*
     * Wait for the connection from a client process.
     * This is an example of a concurrent server.
    */
    clilen = sizeof(cli_addr);
    /* ignoring the waring that the third argument must be of `socklen_t` type, rather than `int` */
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
      perror("server: accept error.");    /* err_dump used here */
      exit(EXIT_FAILURE);
    }

    if ((childpid = fork()) < 0) {
      perror("server: fork error");       /* err_dump used here */
    } else if (childpid == 0) {   /* child process */
      close(sockfd);
      str_echo(newsockfd);
      exit(EXIT_SUCCESS);
    }

    close(newsockfd);     /* parent process */
  }

  exit(EXIT_SUCCESS);
}
