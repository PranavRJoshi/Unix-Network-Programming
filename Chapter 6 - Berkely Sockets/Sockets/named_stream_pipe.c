/*
 * Create a named stream pipe.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>     /* for unlink(), */
#include <strings.h>    /* for bzero(), */

/*
 * C89 function definition:
 *
 *  int ns_pipe (name, fd)
 *  char *name;
 *  int  fd[2]; {
 *    ...
 *  }*/

int ns_pipe (char *name, int fd[static 2]) {
  int                 len;
  struct sockaddr_un  unix_addr;

  // if (s_pipe(fd) < 0) {
  //   return (-1);
  // }
  
  /*
   * Explicit definition for s_pipe (as shown in unnamed_stream_pipe.c).
  */
  if ( socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
    return (-1);
  }

  unlink(name);     /* remove the name, if it already exists. */

  bzero((char *) &unix_addr, sizeof(unix_addr));

  unix_addr.sun_family = AF_UNIX;
  strcpy(unix_addr.sun_path, name);
  #ifdef SUN_LEN 
  len = SUN_LEN(&unix_addr);
  #else 
  len = strlen(unix_addr.sun_path) + sizeof(unix_addr.sun_family);
  #endif

  if (bind(fd[0], (struct sockaddr *) &unix_addr, len) < 0) {
    return (-1);
  }

  return (0);
}
