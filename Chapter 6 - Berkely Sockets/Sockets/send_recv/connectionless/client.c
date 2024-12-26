#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

extern int errno;

#define SERVER_ADDRESS    "127.0.0.1"
#define SERVER_PORT       6969
#define CLIENT_PORT       6970

#define STRINGSIZE(x)     sizeof(x)

int main (void) {

  int client_sfd;

  char    message[256]  = "Hello, UDP Server!";
  size_t  message_len   = strlen(message);

  if ((client_sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("[CLIENT ERROR] Failed to create a connectionless socket.");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in  client_info;

  inet_aton(SERVER_ADDRESS, &(client_info.sin_addr));
  client_info.sin_port    = htons(CLIENT_PORT);
  client_info.sin_family  = AF_INET;

  if (bind(client_sfd, (struct sockaddr *) &client_info, sizeof(struct sockaddr_in)) < 0) {
    perror("[SERVER INFO] Failed to bind the server socket with the specified local address and process.");
  }

  struct sockaddr_in server_info;

  inet_aton(SERVER_ADDRESS, &(server_info.sin_addr));
  server_info.sin_port    = htons(SERVER_PORT);
  server_info.sin_family  = AF_INET;

  fprintf(stdout, "[LOG] Client Process/Port: 0x%X, Server Process/Port: 0x%X\n", client_info.sin_port, server_info.sin_port);

  ssize_t message_status;

  if ((message_status = sendto(client_sfd, message, message_len, 0, (struct sockaddr *) &server_info, sizeof(struct sockaddr_in))) < 0) {
    perror("[CLIENT ERROR] Failed to send message to the server.");
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "[LOG] Size of data sent: %lu\n", message_status);

  if (message_status == 0) {
    perror("[CLIENT ERROR] 0 bytes transmitted to the server.");
    exit(EXIT_FAILURE);
  }

  close(client_sfd);

  exit(EXIT_SUCCESS);
}
