#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <net/route.h>      /* SIOCADDRT and SIOCDELRT macro (not really) defined here */
#include <net/if.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socketvar.h>

/*
 * ioctl function signature:
 *
 *    int ioctl (int fd, unsigned long request, char *arg);
 *
*/

/*
 * Based on musl's C library, the structure definition for `struct rtentry` is as follows:
 *
 * Link: https://git.musl-libc.org/cgit/musl/tree/include/net/route.h
 * 
 *  struct rtentry {
 *    unsigned long int       rt_pad1;
 *    struct sockaddr         rt_dst;
 *    struct sockaddr         rt_gateway;
 *    struct sockaddr         rt_genmask;
 *    unsigned short int      rt_flags;
 *    short int               rt_pad2;
 *    unsigned long int       rt_pad3;
 *    unsigned char           rt_tos;
 *    unsigned char           rt_class;
 *    short int               rt_pad4[sizeof(long)/2-1];
 *    short int               rt_metric;
 *    char                    *rt_dev;
 *    unsigned long int       rt_mtu;
 *    unsigned long int       rt_window;
 *    unsigned short int      rt_irtt;
 *  };
 * 
 * Also, the macro definition for SIOCADDRT and SIOCDELRT is found as:
 *
 * Link: https://git.musl-libc.org/cgit/musl/tree/include/sys/ioctl.h
 *
 *  #define SIOCADDRT          0x890B
 *  #define SIOCDELRT          0x890C
 *  #define SIOCRTMSG          0x890D
 *
 * Unfortunately, seems like the macos SDK for routing seems vague. Most I can find is their own documentation of route (4) manual 
 * page, along with netintro (4) manual page.
*/

int main (void) {

  int sockfd;

  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket error: failed to initialize a stream socket supporting the Internet address family");
    return (-1);
  }

  /*
   * Set to non-blocking mode.
  */
  int set_non_blocking = 1;
  if (ioctl(sockfd, FIONBIO, (char *) &set_non_blocking) < 0) {
    perror("ioctl error: failed to set the socket into non-blocking mode");
    return (-1);
  }

  /*
   * Set asynchronous I/O.
  */
  int set_async_io = 1;
  if (ioctl(sockfd, FIOASYNC, (char *) &set_async_io) < 0) {
    perror("ioctl error: failed to set the socket into asynchronous i/o mode");
    return (-1);
  }

  int get_flags;

  /*
   * Get the flags for the associated file (socket) descriptor.
  */
  if ( (get_flags = fcntl(sockfd, F_GETFL)) < 0) {
    perror("fcntl error: failed to get the flags for the given socket descriptor");
    return (-1);
  }

  /*
   * Check if the socket descriptor has the non-blocking mode set.
  */
  if (get_flags & FNDELAY) {
    fprintf(stdout, "log: The given socket descriptor is set as non-blocking.\n");
  }

  /*
   * Check if the socket descriptor has the asynchronous i/o mode set.
  */
  if (get_flags & FASYNC) {
    fprintf(stdout, "log: The given socket descriptor is set for asynchronous i/o.\n");
  }

  int set_pgrpid;
  set_pgrpid = getpgrp();

  if (ioctl(sockfd, SIOCSPGRP, (char *) &set_pgrpid) < 0) {
    perror("ioctl error: failed to set the process group id for the given socket descriptor");
    return (-1);
  }

  int get_pgrpid;

  if (ioctl(sockfd, SIOCGPGRP, (char *) &get_pgrpid) < 0) {
    perror("ioctl error: failed to fetch the process group id for the given socket descriptor");
    return (-1);
  }

  fprintf(stdout, "log: The process group ID associated with sockfd is: %d\n", get_pgrpid);

  /*
   * Get the ethernet interface information (en0) and fetch the IP address.
  */
  struct sockaddr_in  *bind_addr;
  struct ifreq        interface_info;

  strncpy(interface_info.ifr_name, "en0", IFNAMSIZ);        /* need to provide the interface name. check `ifconfig (1)` command */

  if (ioctl(sockfd, SIOCGIFADDR, (char *) &interface_info) < 0) {     /* if interface name is not provided, error occurs. */
    perror("ioctl error: failed to fetch the interface address");
    return (-1);
  }

  bind_addr = (struct sockaddr_in *) &interface_info.ifr_ifru.ifru_addr;

  fprintf(stdout, "log: en0 IP address name: %s\n", inet_ntoa(bind_addr->sin_addr));
  fprintf(stdout, "log: the size of `struct ifreq` is: %zu\n", sizeof(interface_info));

  return (0);
}
