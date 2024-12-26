/*
 * Description. According to the text:
 *
 *  MISC: The function prototype mentioned in text is: int rresvport (int *aport);
 *
 *      ```
 *      The function [rresvport] creates an Internet stream socket and `bind`s a reserved port to the socket. The socket descriptor 
 *      is returned as the value of the function, unless an error occurs, in which case -1 is returned.
 *
 *      Note that the argument to this function is the address of an integer (a value-result argument), not an integer value. The 
 *      integer pointed to by `aport` is the first port number that the function attempts to `bind`.
 *
 *      The caller typically initializes the starting port number to `IPPORT_RESERVED` - 1. (The value of the constant `IPPORT_RESERVED`
 *      is defined to be 1024 in <netinet.h>.)
 *
 *      If this `bind` fails with an `errno` of `EADDRINUSE`, then the function decrements the port number and tries again. If it 
 *      finally reaches port 512 and finds it already in use, it sets `errno` to EAGAIN, and returns -1.
 *
 *      If this function returns successfully, it not only returns the socket as the value of the function, but the port number is also 
 *      returned in the location pointed to by `aport`.
 *      ```
*/

#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/errno.h>

extern int errno;

int my_rresvport      (int *chosen_resv_port);
int my_rresvport_udp  (int *chosen_resv_port);

int main (int argc, char **argv) {
  
  if (argc != 3) {
    fprintf(stderr, "usage: ./rresvport <port-number> -[t|u]\n");
    exit(EXIT_FAILURE);
  }

  char tloption;    /* transport layer option */

  if (sscanf(argv[2], "-%c", &tloption) < 1) {
    fprintf(stderr, "%s error: transport layer option missing (syntax: -[t|u]).\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int sockfd = -1;
  int desired_port = atoi(argv[1]);

  fprintf(stderr, "[errno log] errno = %d\n", errno);   /* atoi (3) states it does not change errno, but when non-digit character is supplied, then errno is set to EINVAL, which is also what strtol (3) sets when it fails to convert the string to long. */

  switch (tloption) {
    case 't':   
      if ( (sockfd = my_rresvport(&desired_port)) < 0) {
        if (errno == EAGAIN) {
          fprintf(stderr, "[rresvport (tcp) error] failed to bind a socket with any of the reserved port (512-1024).\n");
          exit(EXIT_FAILURE);
        } else if (errno > 0) {
          fprintf(stderr, "[rresvport (tcp) error] failed to create stream socket binded with port %d. errno: %d\n", desired_port, errno);
          exit(EXIT_FAILURE);
        }
      }
      break;
    case 'u':
      if ( (sockfd = my_rresvport_udp(&desired_port)) < 0) {
        if (errno == EAGAIN) {
          fprintf(stderr, "[rresvport (udp) error] failed to bind a socket with any of the reserved port (512-1024).\n");
          exit(EXIT_FAILURE);
        } else if (errno > 0) {
          fprintf(stderr, "[rresvport (udp) error] failed to create datagram socket binded with port %d. errno: %d\n", desired_port, errno);
          exit(EXIT_FAILURE);
        }
      }
      break;
    default:
      fprintf(stderr, "%s error: invalid argument for transport layer option.\n", argv[0]);
      exit(EXIT_FAILURE);
  }

  /* start looping */
  /*
   * When the socket is not `listen`ed, the process which has binded the network address (any Internet interface in this case) will
   * have the status as `CLOSED` which can be verified by using the command: `lsof -i :<port-number>`. To make the port listen to 
   * connection, uncomment the code below which, by default, listens to 1 concurrent client.
  */
  // listen(sockfd, 1);
  while (sockfd > 0) {
    ;
  }

  exit(EXIT_SUCCESS);
}

/*
 *  my_rresvport: Takes the desired port number which must be (*crport >= 512) and (*crport <= 1024). Creates a new socket descriptor
 *                which uses the stream protocol (TCP) used for connecting Internet address family. The socket is binded with a 
 *                network address from the Internet family. We bind with the system's network interface (0.0.0.0) with the port 
 *                `crport` which may be modified if the first port number (sent by the caller) was already in use. May return 
 *                -1 on following conditions:
 *
 *                  1. 512 <= *crport > 1024.
 *                  2. stream socket used for Internet address family could not be created.
 *                  3. bind returned errors apart from EADDRINUSE.
 *                  4. Checked all possible reserved ports but failed to `bind` with one.
 *
 *                Returns the socket descriptor upon successful call.
*/
int my_rresvport (int *crport) {
  int sockfd;     /* socket descriptor which will be returned. */
  int original_desired_port = *crport;

  if (*crport > 1024) {
    fprintf(stderr, "[my_rresvport error] supplied the desired port to be > 1024. port: %d can be automatically assigned by system. Note that if the port is > 5000, the system doesn't automatically assign it.\n", *crport);
    return (-1);
  }

  if (*crport < 512) {
    fprintf(stderr, "[my_rresvport error] supplied the desired port to be < 512. 1-255 are ports used by standard Internet applications. 256-511 are considered reserved by 4.3BSD, but they aren't used by standard Internet application, and they are never allocated by rresvport. Hence, port: %d cannot be assigned.\n", *crport);
    return (-1);
  }

  struct sockaddr_in      sbai;         /* sbai: socket bind address info */
  socklen_t               socklen;      /* size of the structure of sockaddr, we know for Internet, but yeah */

  socklen = sizeof(sbai);

  bzero((char *) &sbai, socklen);

  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("my_rresvport: failed to create an Internet stream socket.");
    return (-1);
  }

  sbai.sin_family       = AF_INET;
  sbai.sin_addr.s_addr  = htonl(INADDR_ANY);

  int bind_ret = -1;    /* initialize, so it doesn't hold any garbage value, although bind will return either 0 or -1. */

  for (; *crport >= 512; (*crport)--) {
    sbai.sin_port = htons(*crport);
    if ( (bind_ret = bind(sockfd, (struct sockaddr *) &sbai, socklen)) < 0) {   /* error occurred */
      if (errno == EADDRINUSE) {
        continue;
      } else {    /* error apart from EADDRINUSE has occurred. Return with corresponding errno set. */
        fprintf(stderr, "[my_rresvport error] bind errno: %d\n", errno);
        // perror("[my_rresvport error] bind error. Message from corresponding errno");
        return (-1);
      }
    }
    if (!bind_ret) {    /* bind returns 0 if successful, !0 is 1... */
      fprintf(stdout, "[my_rresvport log] connected to INADDR_ANY with port %d\n", *crport);
      return sockfd;
    }
  }

  /* we should only be here when the function has attempted to bind with ports from 1024 to 512 and failed. */
  fprintf(stderr, "[my_rresvport error] all reserved ports (min: 512 - desired: %d) in use.\n", original_desired_port);
  errno = EAGAIN;
  return (-1);
}

/*
 *  my_rresvport_udp: UDP alternative to my_rresvport.
*/
int my_rresvport_udp (int *crport) {
  int sockfd;     /* socket descriptor which will be returned. */
  int original_desired_port = *crport;

  if (*crport > 1024) {
    fprintf(stderr, "[my_rresvport_udp error] supplied the desired port to be > 1024. %d can be automatically assigned by system. Note that if the port is > 5000, the system doesn't automatically assign it.\n", *crport);
    return (-1);
  }

  if (*crport < 512) {
    fprintf(stderr, "[my_rresvport_udp error] supplied the desired port to be < 512. 1-255 are ports used by standard Internet applications. 256-511 are considered reserved by 4.3BSD, but they aren't used by standard Internet application, and they are never allocated by rresvport. Hence, %d cannot be assigned.\n", *crport);
    return (-1);
  }

  struct sockaddr_in      sbai;         /* sbai: socket bind address info */
  socklen_t               socklen;      /* size of the structure of sockaddr, we know for Internet, but yeah */

  socklen = sizeof(sbai);

  bzero((char *) &sbai, socklen);

  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("my_rresvport_udp: failed to create an Internet stream socket.");
    return (-1);
  }

  sbai.sin_family       = AF_INET;
  sbai.sin_addr.s_addr  = htonl(INADDR_ANY);

  int bind_ret = -1;    /* initialize, so it doesn't hold any garbage value, although bind will return either 0 or -1. */

  for (; *crport >= 512; (*crport)--) {
    sbai.sin_port = htons(*crport);
    if ( (bind_ret = bind(sockfd, (struct sockaddr *) &sbai, socklen)) < 0) {   /* error occurred */
      if (errno == EADDRINUSE) {
        continue;
      } else {    /* error apart from EADDRINUSE has occurred. Return with corresponding errno set. */
        fprintf(stderr, "[my_rresvport_udp error] bind errno: %d\n", errno);
        return (-1);
      }
    }
    if (!bind_ret) {    /* bind returns 0 if successful, !0 is 1... */
      fprintf(stdout, "[my_rresvport_udp log] connected to INADDR_ANY with port %d\n", *crport);
      return sockfd;
    }
  }

  /* we should only be here when the function has attempted to bind with ports from 1024 to 512 and failed. */
  fprintf(stderr, "[my_rresvport_udp error] all reserved ports (min: 512 - desired: %d) in use.\n", original_desired_port);
  errno = EAGAIN;
  return (-1);
}
