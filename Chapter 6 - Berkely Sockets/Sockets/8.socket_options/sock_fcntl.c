#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main (void) {
  int sockfd;

  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket error: can't open socket");
    return (-1);
  }

  /*
   * Set the socket F_SETOWN. (set the process ID to receive SIGIO or SIGURG for the socket `sockfd` operations).
  */

  if ( fcntl(sockfd, F_SETOWN, getpid()) < 0) {
    perror("fcntl error: F_SETOWN failed to set process group ID for sockfd");
    return (-1);
  }

  /*
   * Check the socket F_GETOWN. (Check if SIGIO and SIGURG can be received on `sockfd` operations).
  */
  int get_f_getown;

  if ( (get_f_getown = fcntl(sockfd, F_GETOWN)) == -1) {
    perror("fcntl error: F_GETOWN failed to retrieve the process/process-group ID");
    return (-1);
  }

  fprintf(stdout, "F_GETOWN for socket `sockfd` = %d\n", get_f_getown);

  /*
   * Check the previous flags for the given file (socket) descriptor.
  */
  int prev_flags;

  if ((prev_flags = fcntl(sockfd, F_GETFL, O_NONBLOCK)) < 0) {
    perror("fcntl error: F_GETFL failed to fetch the flags for the socket descriptor");
    return (-1);
  }

  printf("The flags variable is: %d (0x%x in hexadecimal)\n", prev_flags, prev_flags);

  return (0);
}
