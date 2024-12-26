#include "common.h"
#include "inet.h"
#include <strings.h>    /* for bzero() */
#include <sys/errno.h>

extern int errno;

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

  /* Get peer information before connecting. (local) */

  struct sockaddr_in      local_info;
  int                     local_info_len = sizeof(local_info);

  bzero((char *) &local_info, sizeof(local_info));

  if (getsockname(sockfd, (struct sockaddr *) &local_info, &local_info_len) < 0) {
    perror("client: unable to fetch the local details.");
    exit(EXIT_FAILURE);
  }

  char *local_addr = inet_ntoa(local_info.sin_addr);


  fprintf(stdout, "[LOG] Information about own (before `connect`ing): \n"               \
                  "[#] Network address: %s\n"                                           \
                  "[#] Network port: %d\n", local_addr, ntohs(local_info.sin_port));

  /* End get peer information. (local) */

  /*
   * Connect to the server.
  */
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("client: can't connect to the server.");     /* err_sys used here */
    exit(EXIT_FAILURE);
  }

  /* Get peer information after connecting. (local) */

  /* zero out the structure before reusing it again */
  bzero((char *) &local_info, sizeof(local_info));

  if (getsockname(sockfd, (struct sockaddr *) &local_info, &local_info_len) < 0) {
    perror("client: unable to fetch the local details.");
    exit(EXIT_FAILURE);
  }

  local_addr = inet_ntoa(local_info.sin_addr);

  fprintf(stdout, "[LOG] Information about own (after `connect`ing): \n"               \
                  "[#] Network address: %s\n"                                           \
                  "[#] Network port: %d\n", local_addr, ntohs(local_info.sin_port));

  /* End get peer information. (local) */

  /* Get peer information. (foreign) */

  struct sockaddr_in  peer_info;
  int                 peer_info_len = sizeof(peer_info);    /* getpeername expects us to provide the size of struct being passed. */

  bzero((char *) &peer_info, sizeof(peer_info));

  /*
   * the third argument to getpeername expects a variable of type `socklen_t` but we won't consider that for now
   * also know that getpeername works on a 5-tuple association socket, which is the one obtained after the 
   * `accept` system call. Also, notice that it is a value-result operation.
   *
   * For a blocking socket descriptor in client end, when the client `connect`s to the server, the socket 
   * contains the foreign address and foreign process too.
  */
  if (getpeername(sockfd, (struct sockaddr *) &peer_info, &peer_info_len) < 0) {
    perror("client: unable to get the server details.");
    exit(EXIT_FAILURE);
  }

  char *peer_addr = inet_ntoa(peer_info.sin_addr);

  fprintf(stdout, "[LOG] Information about connected server: \n"          \
                  "[+] Network address: %s\n"                            \
                  "[+] Network port: %d\n", peer_addr, ntohs(peer_info.sin_port));

  /* End get peer information. (foreign) */

  /* 
   * Header using `writev` system call.
  */
  char protocol_name[10] = "UNP";
  struct header_format header_message;
  header_message.type     = 0;
  header_message.version  = 1;
  strncpy(header_message.protocol_name, protocol_name, sizeof(protocol_name));

  char dummy_message[128] = "Header, I/O vectors!";
  char response[128];

  struct iovec first_message[2];
  first_message[0].iov_base = (char *) &header_message;
  first_message[0].iov_len  = sizeof(header_message);

  first_message[1].iov_base = dummy_message;
  first_message[1].iov_len  = strlen(dummy_message);

  if (writev(sockfd, &first_message[0], 2) < 0) {
    perror("client: header type error.");
    exit(EXIT_FAILURE);
  }

  /* check for version response */
  if (recv(sockfd, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "[recv error] Errno: %d\n", errno);
    perror("client: failed to receive header response from server.");
    exit(EXIT_FAILURE);
  }

  if (strncmp(response, VERSION_ERROR, sizeof(VERSION_ERROR)) == 0) {
    fprintf(stderr, "client: version error received. Exiting.");
    shutdown(sockfd, 2);
    exit(EXIT_FAILURE);
  }

  if (strncmp(response, NO_VERSION_ERROR, sizeof(VERSION_ERROR)) == 0) {
    fprintf(stdout, "[LOG] Connected to the server which has same header version!\n");
  }

  /* check for protocol name response */
  if (recv(sockfd, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "[recv error] Errno: %d\n", errno);
    perror("client: failed to receive header response from server.");
    exit(EXIT_FAILURE);
  }

  if (strncmp(response, PROTO_NAME_ERR, sizeof(PROTO_NAME_ERR)) == 0) {
    fprintf(stderr, "client: invalid protocol received. Exiting.");
    shutdown(sockfd, 2);
    exit(EXIT_FAILURE);
  }

  if (strncmp(response, NO_PROTO_NAME_ERR, sizeof(NO_PROTO_NAME_ERR)) == 0) {
    fprintf(stdout, "[LOG] Connected to the server which has same protocol!\n");
  }
  
  /* Header end */

  str_cli(stdin, sockfd);       /* do it all */

  /*
   * second arg (howto) for shutdown:
   *    - SHUT_RD:  0   // shut down reading endpoint 
   *    - SHUT_WR:  1   // shut down writing endpoint
   *    - SHUTRDWR: 2   // shut down both endpoints.
  */
  if (shutdown(sockfd, 2) < 0) {
    perror("client: failed to close down socket's read/recv and write/send endpoints");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
