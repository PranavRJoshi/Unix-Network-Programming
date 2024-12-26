/*
 *  Before getting started, I'd like to write out some of the points that were mentioned in the text. Before mentioning 
 *  the points, let's look at the function signature of {get|set}sockopt:
 *  
 *      int getsockopt (int sockfd, int level, int optname, char *optval, int *optlen);
 *
 *      int setsockopt (int sockfd, int level, int optname, char *optval, int optlen);
 *
 *  Realize that the last argument to `getsockopt`--`optlen`--is a value-result argument.
 *
 *  Now, moving onto the points: 
 *  
 *  1.  The `sockfd` argument *must* refer to an open socket descriptor.
 *
 *  2.  The `level` specifies who in the system is to interpret the option: the general socket code, the TCP/IP code, or the XNS code.
 *
 *  3.  The `optval` argument is a pointer to a user variable from which an option value is set by `setsockopt`, or into which an 
 *      option value is returned by `getsockopt`. Although this argument is type coerced into a `char *`, none of the supported 
 *      options are character values. For instance, if we want to get/set the option "SO_LINGER", we must pass in a variable which
 *      has the type `struct linger *`, which can be obtained either using the `&` operator or by simply using a pointer. 
 *      And yeah, the pointer to variable `struct linger` will be passed as argument to `optval`.
 *
 *  4.  The `optlen` argument to `setsockopt` specifies the size of the variable. The `optlen` argument to `getsockopt` is a 
 *      value-result parameter that we set to the size of `optval` variable before the call. This size is then set by the system on 
 *      return to specify the amount of data stored into the `optval` variable. 
 *
 *      The text states that, "This ability to handle variable-length options is only used by a single option currently: `IP_OPTIONS`.
 *      This might not be the case for newer machines, but I'll not be fact-checking this.
 *
 *  5.  The `optname` argument refers to one of many options that we can get/set, including IP_OPTIONS (for IPPROTO_IP), TCP_MAXSEG 
 *      (for IPPROTO_TCP), SO_KEEPALIVE (for SOL_SOCKET), and so on. Do refer to page 314 of the text to view all the listed 
 *      `optname` column, and other columns.
 *
 *  One more thing regarding the table in page 314, there is a column "flag", which specifies if the option is a flag option. When 
 *  calling `getsockopt` for these flag options, `optval` is an integer. The value returned in `optval` is zero if the option is 
 *  disabled, or nonzero if the option is enabled. Similarly, `setsockopt` requires a nonzero `optval` to turn the option on, and 
 *  a zero value to turn the option off. Also, notice that all the options which has the flag column enabled (indicated
 *  by a black dot in the text), the corresponding "Data type" column has the value `int`. For columns which does not indicate the 
 *  "flag", the option is used to pass a value of the specified data-type between the user process and the system.
*/

/*
 * Example of getsockopt() and setsockopt()
*/

#include <sys/socket.h>       /* for SOL_SOCKET and SO_XX values */
#include <netinet/in.h>       /* for IPPROTO_TCP */
#include <netinet/tcp.h>      /* for TCP_MAXSEG */
#include <netinet/ip.h>       /* options for IP_OPTIONS, and more. Won't be covered here. */
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DUMMYPORT         6969

int main (void) {

  int sockfd;             /* socket descriptor */
  int maxseg;             /* hold the value of TCP_MAXSEG `optname` */
  int sendbuff;           /* buffer containig value to be stored as SOL_SNDBUF `optname` */
  int optlen;             /* holds the size of indirected `optval` object, both use int as "Data type", so sizeof(int) */
  int getsendbuff;        /* variable to store the SO_SENDBUF `optname` */

  char *strcpy(char *, const char *);

  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket error: can't open socket");
    return (-1);
  }

  /*
   * Fetch and print the TCP maximum segment size.
  */

  optlen = sizeof(maxseg);
  if (getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, (char *) &maxseg , (socklen_t *) &optlen) < 0) {
    perror("getsockopt error: TCP_MAXSEG value not fetched");
    return (-1);
  }

  fprintf(stdout, "TCP_MAXSEG = %d\n", maxseg);

  /*
   * Set the send buffer size, then fetch it and print its value.
  */

  sendbuff = 0x4000;
  if ( setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *) &sendbuff, sizeof(int)) < 0 ) {
    perror("setsockopt error: SO_SNDBUF value not set");
    return (-1);
  }

  optlen = sizeof(getsendbuff);

  if ( getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *) &getsendbuff, (socklen_t *) &optlen) < 0 ) {
    perror("getsockopt error: SO_SNDBUF value not fetched");
    return (-1);
  }

  fprintf(stdout, "SO_SNDBUF = %d\n", getsendbuff);

  /*
   * Get the receive buffer size.
  */
  int rcvbufsize;
  int sizeof_so_rcvbuf = sizeof(rcvbufsize);

  if ( getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &rcvbufsize, (socklen_t *) &sizeof_so_rcvbuf) < 0 ) {
    perror("getsockopt error: SO_RCVBUF value not fetched");
    return (-1);
  }

  fprintf(stdout, "SO_RCVBUF = %d\n", rcvbufsize);

  /*
   * Get the current socket debug information.
  */

  int debug_flag;
  int sizeof_so_debug = sizeof(debug_flag);

  if ( getsockopt(sockfd, SOL_SOCKET, SO_DEBUG, (char *) &debug_flag, (socklen_t *) &sizeof_so_debug) < 0 ) {
    perror("getsockopt error: SO_RCVBUF value not fetched");
    return (-1);
  }

  fprintf(stdout, "SO_DEBUG = %d\n", debug_flag);

  /*
   * Get the tcp no delay flag.
  */

  int nodelay_flag;
  int sizeof_no_delay = sizeof(nodelay_flag);

  if ( getsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &nodelay_flag, (socklen_t *) &sizeof_no_delay) < 0) {
    perror("getsockopt error: SO_RCVBUF value not fetched");
    return (-1);
  }

  fprintf(stdout, "TCP_NODELAY = %d\n", nodelay_flag);

  /*
   * Get the IP Options.
  */

  struct ip_opts getopts;
  int get_ip_opts_len = sizeof(getopts);

  /*
   * bind the socket with IN_ADDR address.
  */
  struct sockaddr_in    bind_addr;
  bind_addr.sin_family      = AF_INET;
  bind_addr.sin_port        = htons(DUMMYPORT);       /* sin_port is 16-bit, hence htons */
  bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);      /* s_addr is typedef for 32-bit, hence htonl */

  if ( bind(sockfd, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) < 0) {
    perror("bind error: unable to bind an address to the socket");
    return (-1);
  }

  /*
   * Check RFC 791 Section 3.1 for insights on IP header format.
  */
  if ( getsockopt(sockfd, IPPROTO_IP, IP_OPTIONS, (char *) &getopts, (socklen_t *) &get_ip_opts_len) < 0) {
    perror("getsockopt error: SO_RCVBUF value not fetched");
    return (-1);
  }

  fprintf(stdout, "get_ip_opts_len = %d\n", get_ip_opts_len);

  /*
   * Check the socket type.
  */
  int socktype;
  int get_socktype_len = sizeof(socktype);

  if ( getsockopt(sockfd, SOL_SOCKET, SO_TYPE, (char *) &socktype, (socklen_t *) &get_socktype_len) < 0) {
    perror("getsockopt error: SO_RCVBUF value not fetched");
    return (-1);
  }

  fprintf(stdout, "SO_TYPE = %d\n", socktype);

  switch (socktype) {
    case 1:
      fprintf(stdout, "SO_TYPE = %d (stream socket)\n", socktype);
      break;
    case 2:
      fprintf(stdout, "SO_TYPE = %d (datagram socket)\n", socktype);
      break;
    case 3:
      fprintf(stdout, "SO_TYPE = %d (raw socket)\n", socktype);
      break;
    default:
      fprintf(stderr, "SO_TYPE name error. SO_TYPE = %d\n", socktype);
      break;    /* cannot fallthrough to anything else, but just for the sake of doing "safe" things */
  }

  /*
   * A quick note about what low-water mark really is as the definition in the text is vague.
   *
   *    ```
   *    A low-water mark is the minimum amount of data that must be available in the socket's buffer for certain operations 
   *    to proceed. More specifically, 
   *      
   *      For **reading**, the socket will notify a process that it can read from the buffer only if the available data is 
   *      equal to or exceeds the low-water mark. (receiving)
   *
   *      For **writing**, it determines when the socket will notify a process that it can send data again (when the buffer has 
   *      enough space to meet or exceed this mark). (sending)
   *
   *    Low-water marks are particularly useful in scenarios where applications prefer to wait until there's a 
   *    sufficient amount of data available before performing an operation, reducing overhead for frequent but small transfers.
   *
   *    For instance, since the SO_RCVLOWAT property returns 1 to object pointed by `optval` and SO_SNDLOWAT returns 2048 
   *    to object pointed by `optval` (both when using `getsockopt` call), what it means is the socket can only be "read-able" 
   *    given that the buffer contains at least 1 byte, and the socket will be "write-able" if there is at-least 2048 bytes 
   *    of free space available in the send buffer. 
   *    ```
   *
   * Apparently, the SO_RCVBUF and SO_SNDBUF effectively serves as the high-water mark. The definition of high-water mark is:
   *
   *    ```
   *    The high-water mark concept exists more implicitly and usually pertains to the maximum capacity of a socket's buffer 
   *    (or queue) for receiving or sending data.
   *
   *      If the buffer reaches its maximum (the "high-water mark"), the socket typically stops accepting more data until the 
   *      buffer drains below this level.
   *
   *    While "high-water mark" isn't always explicitly configurable, the buffer size (configured with options like SO_RCVBUF
   *    and SO_SNDBUF) effectively serves as a high-water mark. If one attempts to send data when the buffer is full, the send 
   *    operation may block (in blocking mode) or fail with error like EAGAIN (in non-blocking mode).
   *    ```
  */

  /*
   * Check the receive low watermark.
  */
  int rcvlow_watermark;
  int get_rcvlow_watermark_len = sizeof(rcvlow_watermark);

  if ( getsockopt(sockfd, SOL_SOCKET, SO_RCVLOWAT, (char *) &rcvlow_watermark, (socklen_t *) &get_rcvlow_watermark_len) < 0) {
    perror("getsockopt error: SO_RCVBUF value not fetched");
    return (-1);
  }

  fprintf(stdout, "SO_RCVLOWAT = %d\n", rcvlow_watermark);

  /*
   * Check the send low watermark.
  */
  int sndlow_watermark;
  int get_sndlow_watermark_len = sizeof(sndlow_watermark);

  if ( getsockopt(sockfd, SOL_SOCKET, SO_SNDLOWAT, (char *) &sndlow_watermark, (socklen_t *) &get_sndlow_watermark_len) < 0) {
    perror("getsockopt error: SO_RCVBUF value not fetched");
    return (-1);
  }

  fprintf(stdout, "SO_SNDLOWAT = %d\n", sndlow_watermark);

  /*
   * Get the socket linger option.
  */
  struct linger linger_options;
  int           get_linger_options_len = sizeof(linger_options);

  if ( getsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char *) &linger_options, (socklen_t *) &get_linger_options_len) < 0) {
    perror("getsockopt error: SO_RCVBUF value not fetched");
    return (-1);
  }

  fprintf(stdout, "SO_LINGER: member l_onoff = %d and member l_linger = %d\n", linger_options.l_onoff, linger_options.l_linger);

  return (0);
}

