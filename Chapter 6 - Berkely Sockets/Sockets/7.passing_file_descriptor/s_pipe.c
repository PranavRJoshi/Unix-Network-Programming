#include "common.h"

int s_pipe(int fd[static 2]) {
  return ( socketpair(AF_UNIX, SOCK_STREAM, 0, fd) );
}
