#include "common.h"

int readline (register int fd, register char *ptr, register int maxlen) {
  int   n, rc;
  char  c;

  for (n = 1; n < maxlen; n++) {
    if ((rc = read(fd, &c, 1)) == 1) { 
      *ptr++ = c;
      if (c == '\n') {
        break;
      }
    } else if (rc == 0) {
      if (n == 1) {
        return 0;
      } else {
        break;
      }
    } else {
      return -1;
    }
  }

  *ptr = 0;
  return n;
}
