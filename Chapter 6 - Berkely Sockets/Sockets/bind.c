#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main (void) {

  int sockfd;

  #ifndef _CHECK_TCP
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  #else
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  #endif

  struct in_addr local_addr;
  inet_aton("127.0.0.1", &local_addr);

  struct sockaddr_in local_details;

  uint16_t port = (uint16_t) 6969;  // can also use unsigned short instead of the type definiton used here, but yeah...

  local_details.sin_family  = AF_INET;
  local_details.sin_addr    = local_addr;
  local_details.sin_port    = htons(port);
  
  if (bind(sockfd, (struct sockaddr *) &local_details, sizeof(local_details)) < 0) {
    perror("[ERROR] Cannot bind the socket with the local address and local process.\n");
    exit(EXIT_FAILURE);
  } else {
    fprintf(stdout, "[SUCCESS] Binded the socket with local address (localhost) and local process (6969), for Internet Address Family.\n");
  }

  #ifdef _CHECK_TCP
  /* 
   * Just for demonstration purpose. I will go into the `listen` system call in details later.
   * For now, just know that this system call will "listen" to the address specified during the `bind` system call
   * which essetially gives the socket descriptor the local address and local process values of the 5-tuple association.
   * The second argument, 1 here, implies that the socket can atmost accept connection from n (1 in this case) clients.
  */
  /*
   * Recall the Concurrent server we defined in earlier chapters. After the parent server process `accept`s a 
   * connection from the client, the server `fork`s to create a child process, and have the parent process deal 
   * with other `accept` system call--if *backlog*, the second argument to listen, is > 1--so it acts as a queue for 
   * pending requests to the server.
   *
   * Also note that the `accept` system call is not responsible for `fork`ing the parent server process to create a 
   * child process to handle the client request. A code fragment like this should suffice:
   *
   *    ```
   *    new_sockfd = accept(...);   // blocks until the server receives a connection request
   *    
   *    if (new_sockfd < 0) { // handle error }
   *
   *
   *    if (new_sockfd > 0) {       // a connection has been established
   *      if (fork() == 0) {        // fork the parent process and check if client
   *        // work on new_sockfd (and also close the parent server's socket (sockfd).
   *        exit(0);
   *      }
   *    }
   *
   *    close(new_sockfd);          //  close the socket descriptor for the server
   *    ```
   * 
   * More information about the `accept` system call below.
  */
  listen(sockfd, 1);

  /*
   * Unlike UDP (connectionless), where, once the client process `connect`s to the server, we can just use the `write` 
   * system call to write to the socket descriptor which will send the "message" to the server, in TCP implementation
   * which is connection-oriented, we first need to "accept" the client using the `accept` system call shown below. 
  */

  /*
   * Socket descriptor and socket address (sockaddr_in) to hold the information about the client process (and address).
   * `accept` for the server used here is to accept a connection, but the socket used by the server is not over-written,
   * rather, a new socket descriptor is returned, which is assigned to `client_fd`. 
   * The half association of the client (client's local address, client's local process/port, and the protocol) is stored 
   * in the `client_addr`. `client_addr_len` is used to specify the size of the "specific" socket address rather than the 
   * generic one (struct sockaddr) used by the system.
  */
  int                 client_fd;
  struct sockaddr_in  client_addr;
  unsigned int        client_addr_len = sizeof(client_addr);

  char message[256];
  int message_len;


  /*
   *
   * `accept` function prototype: int accept (int sockfd, struct sockaddr *peer, int *addrlen);
   *
   * Note that `accept`, as I'm aware, is a blocking system call (it apparently is), so, the process is not trigerred unless 
   * a client attempts to connect to the server. 
   *
   * If the socket was defined to be non-blocking, according to the manual, the `accept` system call would assign EWOULDBLOCK 
   * to `errno` and return -1 if there are no process attempting to make a connection.
  */
  /*
   * `accept` for a connection-oriented server will block the caller until the socket descriptor (sockfd)--which has an address 
   * and process/port associated with it (from the `bind` system call)--receives a request from the client, restores the process 
   * and stores the client's information, such as client address and client process/port in the structure `peer` (`client_addr` below) 
   * and `addrlen` is used to specify the size of the structure. 
   *
   * `addrlen` is also called a "value-result" argument: the caller sets its value before the system call, and the system call stores a
   * result in the variable. Often these value-result arguments are integers that the caller sets to the size of the buffer, with the 
   * system call changing this value on the return to the actual amount of data stored in the buffer. 
  */
  if ((client_fd = accept(sockfd, (struct sockaddr *) &client_addr, &client_addr_len)) > 0) {
    fprintf(stdout, "[INFO] Accepted a client: \n"              \
                    "[INFO] Client address: %s \n"              \
                    "[INFO] Client process: 0x%X \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    message_len = read(client_fd, message, sizeof(message));
    write(1, message, message_len);
  }

  #else /* !TCP */

  /*
   * Even for a connectionless server, we need to listen to the addresses binded to our socket descriptor (sockfd) to 
   * receive any packets whose "delivery" address is the one described in `local_details` structure which sockfd 
   * associates itself with.
  */
  listen(sockfd, 1);

  char  message[256];
  int   message_len;

  while (read(sockfd, message, sizeof(message)) <= 0) {
    ;
  }

  message_len = strlen(message);

  write(1, message, message_len);

  #endif

  close(sockfd);

  return 0;
}
