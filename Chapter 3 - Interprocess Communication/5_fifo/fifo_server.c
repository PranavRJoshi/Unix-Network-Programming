#include "fifo.h"
#include "err_routine.h"

#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>

/*
 * Usage: To make sure that the fifo is first created, the server needs to be executed first.
 *        Hence, to make sure of that, the flow of process should be:
 *        1.  First make the program using `make main`
 *        2.  Next, open a terminal and execute server `./server &`
 *        3.  Finally, open another terminal and execute `./client`
 *        4.  To check if the program is working, enter a filename in the current directory, for example, `fifo.h` or `Makefile`
 *        5.  The program should print out the contents of the provided file, and termiate.
*/
int main (void) {

  int readfd, writefd;

  /*
   * Create the FIFOs, then open them - one for reading and one for writing
  */

  if ( (mknod(FIFO1, S_IFIFO | PERMS, 0) < 0) && (errno != EEXIST)) {
    err_sys("can't create fifo: %s", FIFO1);
  }

  if ( (mknod(FIFO2, S_IFIFO | PERMS, 0) < 0) && (errno != EEXIST)) {
    unlink(FIFO1);          /* Incase FIFO2 was failed to be created and not that it already existed, unlink/remove the FIFO1 */
    err_sys("can't create fifo: %s", FIFO2);
  }

  if ( (readfd = open(FIFO1, 0)) < 0) {
    err_sys("client: can't open write fifo: %s", FIFO1);
  }

  if ( (writefd = open(FIFO2, 1)) < 0) {
    err_sys("client: can't open read fifo: %s", FIFO2);
  }

  server(readfd, writefd);

  close(readfd);
  close(writefd);

  exit(EXIT_SUCCESS);
}

