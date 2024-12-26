#include "unix.h"
#include "common.h"

int main (int argc, char **argv) {
  
  int                     sockfd, newsockfd, clilen, childpid, servlen;
  struct sockaddr_un      cli_addr, serv_addr;

  pname = argv[0];      /* process name */

  /*
   * Open a socket (a UNIX domain stream socket).
  */
  if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("server: can't open stream socket.");        /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Bind our local address so that the client can send to us.
  */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, UNIXSTR_PATH);
  #ifdef SUN_LEN    /* Returns the actual size of an initialized sockaddr_un */
  servlen = SUN_LEN(&serv_addr);
  #else   /* !SUN_LEN(<pointer to `struct sockkaddr_un`>) */
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);    /* used in text */
  #endif

  if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("server: can't bind local address");       /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  listen(sockfd, 5);

  for(;;) {
    /*
     * Wait for a connection from a client process. 
     * This is an exmple of a concurrent server.
    */
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
      perror("server: accept error");                 /* err_dump used here */
      exit(EXIT_FAILURE);
    }
    
    if ((childpid = fork()) < 0) {    /* system call error */
      perror("server: fork error");       /* err_dump used here */
      exit(EXIT_FAILURE);
    } else if (childpid == 0) {       /* child process */
      close(sockfd);                        /* close original socket */
      str_echo(newsockfd);                  /* process the request */
      exit(EXIT_SUCCESS);
    }

    close(newsockfd);           /* parent process */

  }

  exit(EXIT_SUCCESS);
}
