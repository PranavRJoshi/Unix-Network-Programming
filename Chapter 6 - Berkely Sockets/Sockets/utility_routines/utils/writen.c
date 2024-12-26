#include "../common.h"

/*
 * writen:  Write 'n' bytes to a descriptor.
 *          Use in place of write() when fd is a stream socket.
*/

int writen (register int fd, register char *ptr, register int nbytes) {
  int nleft, nwritten;

  nleft = nbytes;

  while (nleft > 0) {
    nwritten = write(fd, ptr, nleft);

    if (nwritten <= 0) {        /* ERROR, errno must be set unless 0 is returned */
      return nwritten;
    }

    nleft -= nwritten;
    ptr   += nwritten;
  }

  return (nbytes - nleft);      /* Notice that upon successful call, nleft will be 0, indicating all nbytes was written */
}
