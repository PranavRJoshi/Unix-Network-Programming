#include "unix.h"
#include "common.h"

int main (int argc, char **argv) {

  int                     sockfd, clilen, servlen;
  // char                 *mktemp();      /* no need to explicitly declare the function pointer which will be linked later */
  struct sockaddr_un      cli_addr, serv_addr;
  
  pname = argv[0];

  /*
   * Fill in the structure "serv_addr" with the address of the server that we want to send to.
  */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, UNIXDG_PATH);
  #ifdef SUN_LEN 
  servlen = SUN_LEN(&serv_addr);
  #else /* !SUN_LEN */
  servlen = sizeof(serv_addr.sun_family) + strlen(serv_addr.sun_path);
  #endif

  /*
   * Open a socket (a UNIX domain datagram socket).
  */
  if ( (sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ) {
    perror("client: can't open datagram socket");     /* err_dump used here */
    exit(EXIT_FAILURE);
  }

  /*
   * Bind a local address for us.
   * In the UNIX domain we have to choose our own name (that should be unique).
   * We'll use `mktemp (3)` to create a unique pathname, based on our process ID.
  */
  bzero((char *) &cli_addr, sizeof(cli_addr));        /* zero out */
  cli_addr.sun_family = AF_UNIX;
  strcpy(cli_addr.sun_path, UNIXDG_TMP);
  /*
   * function declaration: char *mktemp (char *template);
   *
   * Some notes:
   *  - The X's in the filename--as in UNIXDG_TEMP--is replaced with alphanumeric characters (A-Z (26), a-z (26), 0-9 (10)) based
   *    on the number of X's. As stated in the manual, mktemp (3), if we have 6 X's, the function call selects one of 
   *    (62 alphanumeric character ** 6 X's) 56,800,235,584 possible name.
   *  - mktemp() seems to be deprecated. This is used only for educational purpose. Furhermore, the manual states that one 
   *    of the possible "bug" from using mktemp() is the fact that the function creates a unique file based on the template 
   *    provided. For a small number of X's, the filename can be guessed. Moreover, this makes a race condition, between testing 
   *    for a file's existence (in the mktemp() function call) and opening it for use (later in the user application) which is 
   *    considered dangerous from a security perspective. Use of `mkstemp()` and `mkostemp()` is encouraged.
   *  - mktemp() guarantees a unique filename based on the X's provided, which will be replaced.
   *  - Passing read-only values (const) or string literal can result in a core dump, because the function call attempts to modify the 
   *    string "constant" that was given. 
   *  - One of the reason we use this is the fact that we need to use unique UNIX pathname when it (client process) binds the 
   *    local address (pathname) to the socket. For UDP or IDP, the local port is specified by 16-bit integer that only has 
   *    significance within the kernel. Therefore, we let the system assign our local port, leaving the responsibility to the system 
   *    to guarantee its uniqueness on the local host.
   *
   *    But in the UNIX domain, we're dealing with pathnames that get turned into actual files by the `bind` system call, so it is 
   *    our responsibility to pick a local address and guarantee its uniqueness on the local host.
   *
   *    While this function could have been delegated to the kernel (as is done with UDP and IDP), since side effects could occur
   *    (such as creating a file in a directory in which the caller doesn't have permission to delete in), the designers chose to 
   *    leave this to the application.
   *
   *  - The text also states why we used exactly 14 characters of pathname as the local address for our client. Refer to page 296 
   *    of the text. Also, shamelessly stole most of the reasoning above from the text :). But, if want basic gist of why this is the 
   *    case, UNIX domain datagrams transmit upto 14 characters (the filename) when the datagram is being sent to the server.
   *
   *    If we choose a pathname consisting of more than 14 characters, only the first 14 characters are sent to the server, hence the 
   *    server can't respond back. If we choose a pathaname with less than 14 characters, extra characters are usually appended, 
   *    which also prevents the server from responding to the client. 
   *
   *    But note that, this is a "feature" in 4.3BSD implementation, so newer systems may have some workaround for this "feature".
  */
  mktemp(cli_addr.sun_path);
  #ifdef SUN_LEN 
  clilen = SUN_LEN(&cli_addr);
  #else       /* !SUN_LEN */
  clilen = sizeof(cli_addr.sun_family) + strlen(cli_addr.sun_path);
  #endif      /* SUN_LEN */

  if (bind(sockfd, (struct sockaddr *) &cli_addr, clilen) < 0) {
    perror("client: can't bind local address");   /* err_dump used here */
    exit(EXIT_FAILURE);
  }
  
  dg_cli(stdin, sockfd, (struct sockaddr *) &serv_addr, servlen);

  close(sockfd);
  unlink(cli_addr.sun_path);    /* remove/unlink UNIXDG_TMP */
  exit(EXIT_SUCCESS);
}
