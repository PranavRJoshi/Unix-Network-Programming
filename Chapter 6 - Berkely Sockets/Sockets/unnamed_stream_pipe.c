/*
 * Create an unnamed stream pipe.
*/

#include <sys/socket.h>
#include <sys/types.h>

/*
 *  C89 function defintion:
 *
 *  int s_pipe (fd)
 *  int fd[2]; {
 *    return ( socketpair(AF_UNIX, SOCK_STREAM, 0, fd) );
 *  }
*/

int s_pipe(int fd[static 2]) {
  return ( socketpair(AF_UNIX, SOCK_STREAM, 0, fd) );
}
