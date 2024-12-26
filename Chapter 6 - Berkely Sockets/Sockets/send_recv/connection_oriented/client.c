#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_ADDR     "127.0.0.1"
#define SERVER_PORT     6969

int main (void) {

  int client_fd;

  if ((client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("[CLIENT ERROR] Failed to create a connection-oriented socket.");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in    server_info;
  socklen_t             server_info_len = sizeof(struct sockaddr_in);

  server_info.sin_family  = AF_INET;
  inet_aton(SERVER_ADDR, &(server_info.sin_addr));
  server_info.sin_port    = SERVER_PORT;

  if (connect(client_fd, (struct sockaddr *) &server_info, server_info_len) < 0) {
    perror("[CLIENT ERROR] Failed to connect to the server with detailes specified.");
    exit(EXIT_FAILURE);
  }

  char  message[256]  = "Hello, TCP Server!";
  int   message_len   = strlen(message);

  int   send_bytes;

  if ((send_bytes = send(client_fd, message, message_len, 0)) < 0) {
    perror("[CLIENT ERROR] Failed to send the message to the server.");
    exit(EXIT_FAILURE);
  }

  printf("[CLIENT LOG] Sent bytes: %d\n", send_bytes);

  close(client_fd);
  
  return 0;
}
