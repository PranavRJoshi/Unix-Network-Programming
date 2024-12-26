#include "common.h"
#include "inet.h"
#include <strings.h>    /* for bzero() */

extern int errno;

#define PROTOCOL_NAME   "UNP"

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

  /* Get peer information. (local) */

  struct sockaddr_in  local_info;
  int                 local_info_len = sizeof(local_info);

  bzero((char *) &local_info, sizeof(local_info));

  if (getsockname(sockfd, (struct sockaddr *) &local_info, &local_info_len) < 0) {
    perror("server: unable to get the local details.");
    exit(EXIT_FAILURE);
  }

  char *local_addr = inet_ntoa(local_info.sin_addr);

  fprintf(stdout, "[LOG] Information about own: \n"          \
                  "[#] Network address: %s\n"                            \
                  "[#] Network port: %d\n", local_addr, ntohs(local_info.sin_port));

  /* End get peer information. (local) */

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
      /* 
       * The code below, when uncommented, generates an error `ENOTCONN` which is expected 
       * since the socket `sockfd` is not "connected", rather it is just a socket which 
       * `listen`s for any potential connection, and when a connection is `accept`ed, 
       * the system will return a new socket descriptor through the return value of 
       * `accept` system call. 
       *
       * We can, ofcourse shut down the reading or writing endpoint of the socket. As the text mentions, 
       * the two endpoints are logically different, hence the data flowing in one direction is logically 
       * independent of the data going in the other direction.
       *
       * close(), on the other hand, just removes the file descriptor on the per-process basis.
      */
      // if (shutdown(sockfd, 2) < 0) {      /* 2 represents SHUT_RDWR */
      //   fprintf(stderr, "[shutdown error] errno: %d\n", errno);
      //   perror("concurrent server: failed to shutdown parent process's socket.");
      //   exit(EXIT_FAILURE);
      // }
      close(sockfd);

      /* Get peer information. (foreign) */

      struct sockaddr_in  peer_info;
      int                 peer_info_len = sizeof(peer_info);

      bzero((char *) &peer_info, sizeof(peer_info));

      /*
       * the third argument to getpeername expects a variable of type `socklen_t` but we won't consider that for now
       * also know that getpeername works on a 5-tuple association socket, which is the one obtained after the 
       * `accept` system call.
      */
      if (getpeername(newsockfd, (struct sockaddr *) &peer_info, &peer_info_len) < 0) {
        fprintf(stderr, "[ERROR] getpeername. Errno: %d\n", errno);
        perror("concurrent server: unable to get the peer details.");
        exit(EXIT_FAILURE);
      }

      char *peer_addr = inet_ntoa(peer_info.sin_addr);

      /* 
       * Header using `writev` system call.
       * Might be one of the worst way to check for headers. Surely there are better ways than this...
      */
      struct header_format header_message;
      char dummy_message[128];

      struct iovec first_message[2];
      first_message[0].iov_base = (char *) &header_message;
      first_message[0].iov_len  = sizeof(header_message);

      first_message[1].iov_base = dummy_message;
      first_message[1].iov_len  = sizeof(dummy_message);

      if (readv(newsockfd, &first_message[0], 2) < 0) {
        perror("concurrent server: header read error.");
        exit(EXIT_FAILURE);
      }

      int is_valid_version  = 0;
      int is_valid_protocol = 0;

      if (header_message.version != 1) {
        const char version_status[128] = VERSION_ERROR;
        is_valid_version = 1;
        if (send(newsockfd, version_status, sizeof(version_status), 0) < 0) {
          perror("concurrent server: cannot send version status message");
          exit(EXIT_FAILURE);
        }
      } else {
        const char version_status[128] = NO_VERSION_ERROR;
        if (send(newsockfd, version_status, sizeof(version_status), 0) < 0) {
          perror("concurrent server: cannot send version status message");
          exit(EXIT_FAILURE);
        }
      }

      if (strncmp(header_message.protocol_name, PROTOCOL_NAME, sizeof(PROTOCOL_NAME)) != 0) {
        is_valid_protocol = 1;
        const char protocol_status[128] = PROTO_NAME_ERR;
        if (send(newsockfd, protocol_status, sizeof(protocol_status), 0) < 0) {
          perror("concurrent server: cannot send protocol message");
          exit(EXIT_FAILURE);
        }
      } else {
        const char protocol_status[128] = NO_PROTO_NAME_ERR;
        if (send(newsockfd, protocol_status, sizeof(protocol_status), 0) < 0) {
          perror("concurrent server: cannot send protocol message");
          exit(EXIT_FAILURE);
        }
      }

      /* kind of confusing, but if the variables below are set, then it means the version and/or protocol was not same. */
      if (is_valid_version || is_valid_protocol) {
        /*
         * Shut down the socket as the client fails to meet version number and/or protocol name.
        */
        if (shutdown(newsockfd, 2) < 0) {
          perror("concurrent server: failed to close down socket with mismatched version number and/or protocol name.");
          exit(EXIT_FAILURE);
        }
        fprintf(stdout, "[LOG] Disconnected from server with mismatched version number and/or protocol name.\n"     \
                        "[-] Network address (disconnected): %s\n"                                                      \
                        "[-] Network port (disconnected): %d\n", peer_addr, ntohs(peer_info.sin_port));
        exit(EXIT_SUCCESS);
      }

      fprintf(stdout, "[LOG] Connected to a client with correct header version and protocol!\n");
      /* Header end */

      fprintf(stdout, "[LOG] Information about the peer (client): \n"          \
                      "[+] Network address: %s\n"                            \
                      "[+] Network port: %d\n", peer_addr, ntohs(peer_info.sin_port));

      /* End get peer information. (foreign) */

      /* Get peer information. (local) */

      struct sockaddr_in  conn_local_info;
      int                 conn_local_info_len = sizeof(conn_local_info);

      bzero((char *) &conn_local_info, sizeof(conn_local_info));

      if (getsockname(newsockfd, (struct sockaddr *) &conn_local_info, &conn_local_info_len) < 0) {
        perror("concurrent server: unable to get the local details.");
        exit(EXIT_FAILURE);
      }

      char *conn_local_addr = inet_ntoa(conn_local_info.sin_addr);

      fprintf(stdout, "[LOG] Information about own (after connection): \n"          \
                      "[#] Network address: %s\n"                            \
                      "[#] Network port: %d\n", conn_local_addr, ntohs(conn_local_info.sin_port));

      /* End get peer information. (local) */

      str_echo(newsockfd);
      exit(EXIT_SUCCESS);
    }

    close(newsockfd);     /* parent process */
  }

  exit(EXIT_SUCCESS);
}
