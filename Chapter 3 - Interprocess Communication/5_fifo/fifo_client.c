#include "fifo.h"
#include "err_routine.h"

#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>

/*
 * Usage: To make sure that the fifo is first created, the server needs to be executed first.
 *        Hence, to make sure of that, the flow of process should be:
 *        1.  First make the program using `make main`
 *        2.  Next, open a terminal and execute server `./server &`
 *        3.  Finally, open another terminal and execute `./client`
 *        4.  To check if the program is working, enter a filename in the current directory, for example, `fifo.h` or `Makefile`
 *        5.  The program should print out the contents of the provided file, and termiate.
 *        6.  The fifo_client before exiting, deletes the fifo created in the `/tmp` directory.
*/
int main (void) {

  int readfd, writefd;

  /*
   * Open the FIFOs. We assume the server has already created them.
  */

  if ( (writefd = open(FIFO1, 1)) < 0) {
    err_sys("client: can't open write fifo: %s", FIFO1);
  }

  if ( (readfd = open(FIFO2, 0)) < 0) {
    err_sys("client: can't open read fifo: %s", FIFO2);
  }

  client(readfd, writefd);

  close(readfd);
  close(writefd);

  /*
   * Delete the FIFOs, now that we're finished.
  */

  if (unlink(FIFO1) < 0) {
    err_sys("client: can't unlink %s", FIFO1);
  }

  if (unlink(FIFO2) < 0) {
    err_sys("client: can't unlink %s", FIFO2);
  }

  exit(EXIT_SUCCESS);
}
