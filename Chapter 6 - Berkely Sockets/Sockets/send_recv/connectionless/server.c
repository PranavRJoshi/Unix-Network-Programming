#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

extern int errno;

#define SERVER_ADDRESS    "127.0.0.1"
#define SERVER_PORT       6969

#define STRINGSIZE(x)     sizeof(x)

int main (void) {

  unsigned long message_id = 0;

  int server_sfd;

  if ((server_sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("[CLIENT ERROR] Failed to create a connectionless socket.");
    exit(EXIT_FAILURE);
  }


  struct sockaddr_in  server_info;

  inet_aton(SERVER_ADDRESS, &(server_info.sin_addr));
  server_info.sin_port    = htons(SERVER_PORT);
  server_info.sin_family  = AF_INET;

  if (bind(server_sfd, (struct sockaddr *) &server_info, sizeof(struct sockaddr_in)) < 0) {
    perror("[SERVER INFO] Failed to bind the server socket with the specified local address and process.");
    exit(EXIT_FAILURE);
  }

  char message_buffer[256];
  struct sockaddr     client_details;
  unsigned int        client_sock_addr_size;
  size_t message_len;

  /*
   * BUG: For some reason, when the server program is up and running and the first client program is executed, the 
   *      wait for `recvfrom` is not terminated even though the client sends the message, in case if the `flag` is set to 
   *      only 0. It works as intended when the first `recvfrom` function contains the `MSG_PEEK` flag. Note that `MSG_PEEK`
   *      flag implies that the content available in the socket decriptor is to be read but the system does not discard the 
   *      data after the `recvfrom` function. (`0` flag discards the content after reading from the socket descriptor.)
   *      If we have the `MSG_PEEK` enabled inside the while loop below, it would never stop as the socket descriptor will always 
   *      have the content left to be read since no discarding takes place.
  */
  if ((message_len = recvfrom(server_sfd, message_buffer, sizeof(message_buffer), MSG_PEEK, &client_details, &client_sock_addr_size)) < 0) {
    perror("[SERVER ERROR] Failed to receive message from the client.");
    exit(EXIT_FAILURE);
  }

  /*
   * BUG: In case that we provide `NULL` as argument to last two parameters in `recvfrom` function above (with `MSG_PEEK` flag),
   *      The server continues to not notice the content in the server's socket descriptor, i.e., the first packet sent to the 
   *      server does not trigger the log which is inside the while loop below.
  */
  fprintf(stdout, "[SERVER LOG] First Peeked Message size: %lu\n", message_len);


  while (1) {
    if ((message_len = recvfrom(server_sfd, message_buffer, sizeof(message_buffer), 0, &client_details, &client_sock_addr_size)) < 0) {
      perror("[SERVER ERROR] Failed to receive message from the client.");
      exit(EXIT_FAILURE);
    }

    if (client_details.sa_family == AF_INET) {
      struct sockaddr_in *cli_det = (struct sockaddr_in *) &client_details;
      char *client_addr = inet_ntoa(cli_det->sin_addr);
      if (message_len > 0) {
        fprintf(stderr, "[LOG] Message to the server was: \n");
        message_buffer[message_len] = '\n';
        message_buffer[message_len + 1] = '\0';
        write(1, message_buffer, strlen(message_buffer));
        fprintf(stderr, "[LOG] Client's Address: %s\n"                \
                        "[LOG] Client's Port: 0x%X\n"                 \
                        "[LOG] Client's Socket Address Size: %u\n"    \
                        "[LOG] Message Length: %lu\n"                 \
                        "[LOG] Message ID: %lu\n",  client_addr, cli_det->sin_port, client_sock_addr_size, message_len, message_id);
        message_id++;
      }
    }
  }

  exit(EXIT_SUCCESS);
}

