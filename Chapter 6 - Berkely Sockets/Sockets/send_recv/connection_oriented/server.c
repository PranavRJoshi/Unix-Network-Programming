#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define SERVER_ADDR     "127.0.0.1"
#define SERVER_PORT     6969

#define MESSAGE_LEN     256

int main (void) {

  int server_fd;

  if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("[SERVER ERROR] Failed to create a connection-oriented socket.");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in    server_info;
  socklen_t             server_info_len = sizeof(struct sockaddr_in);

  server_info.sin_family  = AF_INET;
  inet_aton(SERVER_ADDR, &(server_info.sin_addr));
  server_info.sin_port    = SERVER_PORT;

  if ((bind(server_fd, (struct sockaddr *) &server_info, server_info_len)) < 0) {
    perror("[SERVER ERROR] Failed to bind the given address to the server process.");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 2) < 0) {
    perror("[SERVER ERROR] Failed to ensure server can listen to 2 clients at a time.");
    exit(EXIT_FAILURE);
  }

  int full_association_sfd;     // full 5-tuple association socket file descriptor.

  struct sockaddr_in    client_details;
  socklen_t             client_details_size;

  bzero(&client_details, sizeof(struct sockaddr_in));     /* function used to fill the structure with zeroes */

  char  message[MESSAGE_LEN + 1];

  while (1) {
    // Recall the value-result argument definition. Yeah, that's why we need to send the pointer to the detail size rather than value...
    full_association_sfd = accept(server_fd, (struct sockaddr *) &client_details, &client_details_size);

    if (full_association_sfd < 0) {
      perror("[SERVER ERROR] Failed to accept client connection.");
      exit(EXIT_FAILURE);
    }

    if (fork() == 0) {
      int recv_size;
      close(server_fd);       /* Child process has no usage of the server socket descriptor. */
      if ((recv_size = recv(full_association_sfd, message, MESSAGE_LEN, 0)) < 0) {
        perror("[SERVER ERROR] Failed to receive any message from the socket descriptor.");
        exit(EXIT_FAILURE);
      }
      if (message[recv_size] != '\0') {
        message[recv_size] = '\0';
      }
      fprintf(stderr, "[LOG MSG_SIZE = %d] Message Received: %s\n", recv_size, message);
      exit(EXIT_SUCCESS);
    }
    
    close(full_association_sfd);    /* For an concurrent server, the "connect"ed socket descriptor is no used by parent process. */
  }

  return 0;
}
