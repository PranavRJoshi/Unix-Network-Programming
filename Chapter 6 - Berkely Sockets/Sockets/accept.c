#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

extern int errno;

#define FREE_AND_NULL(malloced_variable)        \
free((malloced_variable));                      \
malloced_variable = NULL

/* By default, the program will act as a client, but with the macro _TCP_SERVER defined, we will `undef` the _TCP_CLIENT */
#define _TCP_CLIENT

#ifdef _TCP_SERVER
#undef _TCP_CLIENT
#endif

// TODO: Make sure to add conditional compilation to function declarations and definitions for server and client.

#ifdef _TCP_SERVER

/* Make sure that the `dotted_address` is not NULL*/
int                             \
__attribute__ ((nonnull(1)))    \
tcp_open      (const char *dotted_address, uint16_t port, uint16_t max_client);

int                             \
tcp_accept    (int socket_descriptor, struct sockaddr **client_association, int **association_size);

void server_sigint(int signal);

void clear_standard_output (void);

#endif

#ifdef _TCP_CLIENT

/* Make sure that the `dotted_address` is not NULL*/
int                             \
__attribute__ ((nonnull(1)))    \
tcp_connect   (const char *dotted_address, uint16_t port);

#endif


int main (void) {

  #ifdef _TCP_CLIENT 
    int client_sockfd;

    const char  *destination_address    = "127.0.0.1";
    uint16_t    foreign_port            = 6969;

    if ((client_sockfd = tcp_connect(destination_address, foreign_port)) < 0) {
      perror("[ERROR] Failed to establish a TCP connection at the specified address and port.");
      exit(EXIT_FAILURE);
    }
    
    fprintf(stdout, "[LOG] Connection established with server of address: %s and process/port: %d\n", destination_address, foreign_port);
  
  #else  /* !_TCP_CLIENT */
    
    signal(SIGINT, server_sigint);

    /* Define the server address and process number that it'll be listening to. */
    int server_sockfd;

    const char  *server_address = "127.0.0.1";
    uint16_t    server_port     = 6969;

    if ((server_sockfd = tcp_open(server_address, server_port, 5)) < 0) {
      perror("[ERROR] Failed to open a TCP connection with the address and port specified");
      exit(EXIT_FAILURE);
    }

    fprintf(stdout, "[LOG] Listening to address: %s on port: %d\n", server_address, server_port);

    struct sockaddr_in  *client_association       = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    int                 *client_association_size  = (int *) malloc(sizeof(int));
    int                 client_association_sockfd;

    for (;;) {

      if ((client_association_sockfd = tcp_accept(server_sockfd, (struct sockaddr **) &client_association, &client_association_size)) < 0) {
        perror("[ERROR] Failed to accept client connection.");
        exit(EXIT_FAILURE);
      }

      if (fork() == 0) {      /* child process */
        close(server_sockfd);

        uint16_t  client_port       = ntohs(client_association->sin_port);
        char      *client_address   = inet_ntoa(client_association->sin_addr);

        /* Log the client and close the process */
        fprintf(stdout, "[SERVER LOG] Information about the client connected:\n"        \
                        "[ADDRESS] %s\n"                                                \
                        "[PROCESS] %d (0x%X in hex)\n",                                 \
                        client_address, client_port, client_port);
        exit(EXIT_SUCCESS);
      }
      
      /* Parent process has no need to access the socket descriptor returned after `accept`ing the connection */
      close(client_association_sockfd);
      
    }

  #endif
  
  exit(EXIT_SUCCESS);
}

#ifdef _TCP_SERVER

int tcp_open (const char *dotted_address, uint16_t port, uint16_t max_client) {

  int socket_descriptor;

  /* Create a socket that runs on the Internet with connection-oriented service (TCP) */
  if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("[ERROR SERVER] Failed to create a TCP socket");
  }

  struct sockaddr_in *server_association;

  server_association = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
  if (server_association == NULL) {
    perror("[ERROR SERVER] Cannot allocate memory for a socket address structure (sockaddr_in) to hold server association");
    return -1;
  }
  
  server_association->sin_family = AF_INET;
  server_association->sin_port   = htons(port);

  inet_aton(dotted_address, &(server_association->sin_addr));

  if (bind(socket_descriptor, (struct sockaddr *) server_association, sizeof(*server_association)) < 0) {
    perror("[ERROR SERVER] Cannot bind the specified address and port.");
    FREE_AND_NULL(server_association);
    close(socket_descriptor);
    return -1;
  }

  if (max_client > 5 || listen(socket_descriptor, max_client) < 0) {
    perror("[ERROR SERVER] Server cannot listen to max clients specified.");
    FREE_AND_NULL(server_association);
    close(socket_descriptor);
    return -1;
  }

  return socket_descriptor;

}

int tcp_accept (int sockfd, struct sockaddr **peer, int **addrlen) {
  
  int new_sockfd;

  if ((new_sockfd = accept(sockfd, *peer, (socklen_t *) addrlen)) < 0) {
    perror("[ERROR SERVER] Cannot accept peer connection.");
    return -1;
  }

  return new_sockfd;
}

void server_sigint (int signal) {
  fprintf(stdout, "\n[WARN] Received the Interrupt Signal (SIGINT) (Signal Number: %d)\n", signal);

  int option;

  fprintf(stdout, "Do you want to close the server? (y/n): ");
  option = getchar();
  
  clear_standard_output();

  switch (option) {
    case 'y': case 'Y':
      // TODO clear out the malloc'd client info and exit
      exit(EXIT_SUCCESS);
    default:
      return;
  }
}

void clear_standard_output (void) {
  while (getchar() != '\n') {
    ;
  }
}

#endif

#ifdef _TCP_CLIENT

int tcp_connect (const char *dotted_address, uint16_t port) {
  
  int socket_descriptor;

  if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("[ERROR CLIENT] Failed to create a TCP socket.");
    return -1;
  }

  struct sockaddr_in *server_info;

  server_info = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));

  if (server_info == NULL) {
    perror("[ERROR CLIENT] Cannot allocate memory for a socket address structure (sockaddr_in) to hold server info");
    return -1;
  }

  server_info->sin_family = AF_INET;
  server_info->sin_port   = htons(port);

  inet_aton(dotted_address, &server_info->sin_addr);

  if (connect(socket_descriptor, (struct sockaddr *) server_info, sizeof(*server_info)) < 0) {
    perror("[ERROR CLIENT] Failed to connect to the address specified.");
    FREE_AND_NULL(server_info);
    close(socket_descriptor);
    return -1;
  }

  return socket_descriptor;

}

#endif
