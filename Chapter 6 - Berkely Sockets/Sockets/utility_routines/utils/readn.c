#include "../common.h"

/*
 * readn: Read 'n' bytes from a descriptor.
 *        Use in place of read() when fd is a stream socket.
*/

int readn (register int fd, register char *ptr, register int nbytes) {
  int nleft, nread;

  nleft = nbytes;

  while (nleft > 0) {
    nread = read(fd, ptr, nleft);

    if (nread < 0) {                /* ERROR, errno must be set */
      return nread;
    } else if (nread == 0) {        /* End Of File */
      break;
    }

    nleft -= nread;                 /* Keep track of number of bytes read, and subtract that from `nleft` to read remaining */
    ptr   += nread;                 /* Advance the pointer to point to the byte where data is not stored, to store more content */
  }

  return (nbytes - nleft);          /* return >= 0 */
}
