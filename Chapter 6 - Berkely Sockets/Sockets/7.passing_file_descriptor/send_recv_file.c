#include "common.h"
#include <string.h>

/*
 *  So, many things that have changed since the time UNP was written (back in 1990). Where do I even begin...
 *
 *  Anyways, lets first start off by looking at the structure of `msghdr` presented in the text:
 *
 *  # Defined in <sys/socket.h>
 *  struct msghdr {
 *    caddr_t         msg_name;             // optional address  
 *    int             msg_namelen;          // size of address
 *    struct iovec    *msg_iov;             // scatter/gather array
 *    int             msg_iovlen;           // # elements in msg_iov
 *    caddr_t         msg_accrights;        // access rights sent/recvd
 *    int             msg_accrightslen;
 *  };
 *
 *  As per the text (Refer to page 301):
 *    `msg_name` and `msg_namelen`: used when socket is not connected, similar to two final arguments to `recvfrom` and `sendto`. 
 *                                  can be NULL.
 *    `msg_iov` and `msg_iovlen`: used for scatter read and gather write, as described for `readv` and `writev`.
 *    `msg_accrights` and `msg_accrightslen`: passing and receiving access rights between processes.
 *
 *    NOTE: Text mentions another member, `flag`, as being same with `send` and `recv` system call, but is not visible in the struct 
 *          defined.
 *
 *  Now, the structure for `msghdr` struct in my machine (and other newer machines, I presume) have the following declaration:
 *
 *  # msghdr in my machine
 *  struct msghdr {
 *    void            *msg_name;      // optional address
 *    socklen_t       msg_namelen;    // size of address
 *    struct          iovec *msg_iov; // scatter/gather array
 *    int             msg_iovlen;     // # elements in msg_iov
 *    void            *msg_control;   // ancillary data, see below
 *    socklen_t       msg_controllen; // ancillary data buffer len
 *    int             msg_flags;      // flags on received message
 *  };
 *
 *  As per the manual, the `msg_control` argument, which has the length of `msg_controllen`, points to a buffer for other protocol 
 *  related message or other miscellaneous ancillary (providing support to primary activity) data. The message are of the form:
 *
 *  # control message
 *  struct cmsghdr {
 *    u_int cmsg_len;           // data byte count, including cmsghdr
 *    int cmsg_level;           // originating protocol
 *    int cmsg_type;            // protocol-specific type
 *    // followed by
 *    // u_char cmsg_data[];
 *  }
 *
 *  Moving on, as per the `recv (2)` manual, open file descriptors are now passed as ancillary data for AF_UNIX domain sockets, with 
 *  `cmsg_level` set to `SOL_SOCKET` and `cmsg_type` set to `SCM_RIGHTS`. Keep note of the use of the word `level`, it usually means the 
 *  protocol suite or normal sockets (raw sockets, as in `SOL_SOCKET`). This comes later when we learn about socket options.
 *
 *  Weirdly, the manual mentions that cmsghdr could be used to "learn of changes in the data-stream in XNS/SPP", but there is no support
 *  for XNS protocol suite containing SPP or PEX or even IDP. [misc] Recall that all the error handling of packet is done in IDP, 
 *  unlike IP which only has error correction for the addresses and not the actual data. Also, IDP demultiplexes the incoming datagrams
 *  based on the port number, since the IDP port number is independent of the actual protocol. Because of this, it is possible for a 
 *  SPP protocol module to receive an ERROR protocol packet whose destination is the port using SPP. [/misc] Another one is that, when 
 *  transferring data through SPP, there are three-level hierarchy in the data being transferred using SPP (unlike TCP, which only 
 *  provides byte stream interface), which are:
 *
 *  1. Bytes are basic entity.
 *  2. A packet is composed of zero or more bytes.
 *  3. A message is composed of one or more packets.
 *
 *  With this, SPP can present three different interfaces to a user process:
 *
 *  1. A byte-stream interface: bytes are delivered to the user process in order.
 *  2. A packet-stream interface: packets are delivered to the user process in order.
 *  3. A reliable-packet interface: packets are delivered to the user process, but they might be out of order.
 *
 *  Possibly, `cmsghdr` is used to detect such changes. For further reading, refer to page 218 of text.
 *  For further reading, refer to RFC 2292, Section 4.3. It explains some of the macros used here such as CMSG_FIRSTHDR, 
 *  CMSG_DATA, CMSG_SPACE as well as CMSG_LEN. Also, one of the author of this RFC is W.R. Stevens :)
*/

/*
 * For reference (regarding cmsghdr structure), shamelessly stole this from the RFC 2292 (Section 4.2) document:
 *    
 *    |<--------------------------- msg_controllen -------------------------->|
 *    |                                                                       |
 *    |<----- ancillary data object ----->|<----- ancillary data object ----->|
 *    |<---------- CMSG_SPACE() --------->|<---------- CMSG_SPACE() --------->|
 *    |                                   |                                   |
 *    |<---------- cmsg_len ---------->|  |<--------- cmsg_len ----------->|  |
 *    |<--------- CMSG_LEN() --------->|  |<-------- CMSG_LEN() ---------->|  |
 *    |                                |  |                                |  |
 *    +-----+-----+-----+--+-----------+--+-----+-----+-----+--+-----------+--+
 *    |cmsg_|cmsg_|cmsg_|XX|           |XX|cmsg_|cmsg_|cmsg_|XX|           |XX|
 *    |len  |level|type |XX|cmsg_data[]|XX|len  |level|type |XX|cmsg_data[]|XX|
 *    +-----+-----+-----+--+-----------+--+-----+-----+-----+--+-----------+--+
 *    ^
 *    |
 *    msg_control
 *    points here
 *
 *  NOTE: The fields shown as "XX" are possible padding, between the cmsghdr structure and the data, and between the 
 *  data and the next cmsghdr structure, if required by the implementation.
 *
 *  NOTE: This diagram shows that control message header may contain more than one ancillary data object that can be sent or 
 *  received using the `sendmsg` and `recvmsg` system call. As we are only using this to send one ancillary data, i.e. the 
 *  file descriptor, we won't be focusing on using it for more than one ancillary objects which can be done with the help 
 *  of CMSG_NXTHDR macro.
*/

int my_sendfile (int sockfd, int fd) {

  struct iovec iov[1];
  struct msghdr msg;
  extern int errno;

  /*
   * According to RFC 2292 Section 4.3.4, definition for CMSG_SPACE is:
   *  
   *    ```
   *            unsigned int CMSG_SPACE(unsigned int length);
   *
   *    This macro is new with this API.  Given the length of an ancillary data object, CMSG_SPACE() returns the space 
   *    required by the object and its cmsghdr structure, including any padding needed to satisfy alignment requirements.
   *    This macro can be used, for example, to allocate space dynamically for the ancillary data.  This macro should 
   *    not be used to initialize the cmsg_len member of a cmsghdr structure; instead use the CMSG_LEN() macro.
   *    ```
   *
   * The implementation as described in the RFC is as follows:
   *
   *    #define CMSG_SPACE(length) ( ALIGN(sizeof(struct cmsghdr)) + \
   *        ALIGN(length) )
   *
   * Here, the length of the ancillary data object is the size of an file descriptor, which is an integer, hence we passed
   *  `sizeof(int)`. 
  */
  char control[CMSG_SPACE(sizeof(int))];
  memset(control, 0, sizeof(control));

  iov[0].iov_base = (char *) 0;
  iov[0].iov_len  = 0;

  msg.msg_iov         = iov;
  msg.msg_iovlen      = 1;
  msg.msg_name        = (caddr_t) 0;      /* caddr_t: typedef for pointer to char (char *) */
  msg.msg_control     = control;          /* text: address of the descriptor */ /* pointer to control message header, see below */
  msg.msg_controllen  = sizeof(control);  /* text: pass 1 descriptor */ /* length of control message header */

  /* control message header, required to send message, or rather file descriptor using {send|recv}msg */
  /*
   * According to RFC 2292, Section 4.3.1, CMSG_FIRSTHDR has the definition as follows:
   *
   *    ```
   *          struct cmsghdr *CMSG_FIRSTHDR(const struct msghdr *mhdr);
   *
   *    CMSG_FIRSTHDR() returns a pointer to the first cmsghdr structure in the msghdr structure pointed to by mhdr.
   *    The macro returns NULL if there is no ancillary data pointed to the by msghdr structure (that is, if either 
   *    msg_control is NULL or if msg_controllen is less than the size of a cmsghdr structure).
   *    ```
   *
   * For my machine, the implementation of CMSG_FIRSTHDR is the same as the one used in the RFC, i.e. 
   *
   *      #define CMSG_FIRSTHDR(mhdr) \ 
   *          ( (mhdr)->msg_controllen >= sizeof(struct cmsghdr) ? \
   *          (struct cmsghdr *)(mhdr)->msg_control : \
   *          (struct cmsghdr *)NULL )
   *
   * Realize that the arrow operator (->) has higher precedence as compared to explicit casting. 
   * Also, msg_control is an array of character/byte, so, 
   *
   *    ```
   *    (struct cmsghdr *)(mhdr)->msg_control
   *    ```
   *
   * will first point to the starting byte of the member `msg_control`, and then only type cast it to a pointer of type `struct cmsghdr *`,
   * whose members are specified above. 
  */
  struct cmsghdr *cmsg  = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level      = SOL_SOCKET;
  cmsg->cmsg_type       = SCM_RIGHTS;
  /*
   * According to RFC 2292, Section 4.3.5, the definition for CMSG_LEN macro is described as:
   *      
   *      ```
   *              unsigned int CMSG_LEN(unsigned int length);
   *
   *      This macro is new with this API.  Given the length of an ancillary data object, CMSG_LEN() returns 
   *      the value to store in the cmsg_len member of the cmsghdr structure, taking into account any padding 
   *      needed to satisfy alignment requirements.
   *      ```
   *
   * The implementation as described in the RFC is as follows:
   *    
   *      #define CMSG_LEN(length) ( ALIGN(sizeof(struct cmsghdr)) + length )
   * 
   * It sure is confusing, as it looks similar to CMSG_SPACE, but the RFC provides a good disclaimer, which is:
   *
   *      ```
   *      Note the difference between CMSG_SPACE() and CMSG_LEN(), shown also in the figure in Section 4.2 (the figure above): 
   *      the former accounts for any required padding at the end of the ancillary data object and the latter 
   *      is the actual length to store in the cmsg_len member of the ancillary data object.
   *      ```
  */
  cmsg->cmsg_len        = CMSG_LEN(sizeof(int));

  /*
   * According to RFC 2292, Section 4.3.3, the definition for CMSG_DATA is as follows:
   *
   *      ```
   *              unsigned char *CMSG_DATA(const struct cmsghdr *cmsg);
   *
   *      CMSG_DATA() returns a pointer to the data (what is called the cmsg_data[] member, even though 
   *      such a member is not defined in the structure) following a cmsghdr structure.
   *      ```
   * 
   * The implementation described in the RFC is as follows:
   *
   *    #define CMSG_DATA(cmsg) ( (u_char *)(cmsg) + \
   *        ALIGN(sizeof(struct cmsghdr)) )
   * 
   * In essence, what this does is, realize that cmsg, which is of `struct cmsg *` type is first type-casted to `unsinged char *`
   * and that pointer is then added by the size of a structure of tag `cmsghdr`. If you don't know much about pointer arithmetic, 
   * let's look at a simple example:
   *
   *        |   Address           |     Data      |
   *        | 0x0000000000000000  |      69       |
   *        | 0x0000000000000004  |     420       |
   *        | 0x0000000000000008  |      0        |
   *        | 0x000000000000000c  |     -10       |
   * 
   * I'm taking 8-bytes for the address as most modern machines uses 32-bits for addresses. Also, I'm assuming the machine uses 32-bits
   * for an integer, hence 4-bytes increment between addresses.
   *
   * Say that a pointer is defined which points to the data `69`, we'll call that variable ptr, which is declared as:
   *    
   *    ```
   *    int *ptr = <address to data 69>;
   *    ```
   * 
   * Now, when we do ptr++ or ++ptr or ptr += 1 or other addition operation, one might think that ptr will now point to 
   * `0x0000000000000001`, but this is not what happens. Pointer arithmetic is used to make the pointer point to next 
   * "valid" address, so an addition of 1 to ptr must point to `0x0000000000000004` (data = 420), because an integer 
   * (in our assumption) takes 4-bytes of data. With the same logic, when we add 2 to `ptr`, the updated `ptr` will 
   * point to data 0, which is in address `0x0000000000000008`.
   *
   * Looking back at the implementation of CMSG_DATA macro, since `cmsg` has been type-casted to `unsigned char *`,
   * and it is added by the size of `struct cmsghdr`, we are essentially moving `sizeof(struct cmsghdr)` bytes from the 
   * initial position of `cmsg`, making it point to the ancillary object data. Recall that characters take one byte, be it
   * signed or unsigned, hence there is `ptr + n * sizeof(<data-type>)`, as `sizeof(<data-type>)` in this 
   * case will be 1 (sizeof(u_char)), `n` is `sizeof(struct cmsghdr)`, `ptr` is `(u_char *) cmsg`.
   *
   * We can also observe that the result is type-casted to `int *` as the ancillary object is an integer. Since the result 
   * is still a pointer, we need to de-reference it, and then only are we able to store the file descriptor.
   *
   * The reason these macros exist is because structures and C in general has many implentation defined behaviors, especially 
   * for structures where the conecpt of "holes" exist. By having portability in mind and to follow POSIX guidelines, the 
   * manufacturers are responsible for provided these macro APIs, which lessens the stress when dealing with these sorts of 
   * stuffs.
  */
  *((int *) CMSG_DATA(cmsg)) = fd;

  if (sendmsg(sockfd, &msg, 0) < 0) {
    return ( (errno > 0) ? errno : 255 );
  }

  return (0);
}

int my_recvfile (int sockfd) {
  int             fd;
  struct iovec    iov[1];
  struct msghdr   msg;
  extern int errno;

  iov[0].iov_base = (char *) 0;
  iov[0].iov_len  = 0;

  char control[CMSG_SPACE(sizeof(int))];
  memset(control, 0, sizeof(control));

  msg.msg_iov         = iov;
  msg.msg_iovlen      = 1;
  msg.msg_name        = (caddr_t) 0;
  msg.msg_control     = control;          /* address of descriptor */ /* pointer to control message header, see below */
  msg.msg_controllen  = sizeof(control);  /* receive 1 descriptor */ /* length of control message header */

  if (recvmsg(sockfd, &msg, 0) < 0) {
    return ( (errno > 0) ? errno : 255 );
  }

  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

  if (cmsg == NULL || cmsg->cmsg_type != SCM_RIGHTS) {
    fprintf(stderr, "Invalid control messsage");
    exit(EXIT_FAILURE);
  }

  fd = *((int *) CMSG_DATA(cmsg));

  return (fd);
}
