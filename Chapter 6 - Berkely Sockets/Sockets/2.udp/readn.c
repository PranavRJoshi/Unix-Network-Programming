#include "common.h"

int readn (register int fd, register char *ptr, register int nbytes) {
  int nleft, nread;

  nleft = nbytes;

  while (nleft > 0) {
    nread = read(fd, ptr, nleft);

    if (nread < 0) {
      return nread;
    } else if (nread == 0) {
      break;
    }

    nleft -= nread;
    ptr   += nread;
  }

  return (nbytes - nleft);
}
