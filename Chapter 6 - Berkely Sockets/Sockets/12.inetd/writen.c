#include "utils.h"

int writen (register int fd, register char *ptr, register int nbytes) {
  int nleft, nwritten;

  nleft = nbytes;

  while (nleft > 0) {
    nwritten = write(fd, ptr, nleft);

    if (nwritten <= 0) {
      return nwritten;
    }

    nleft -= nwritten;
    ptr   += nwritten;
  }

  return (nbytes - nleft);
}
