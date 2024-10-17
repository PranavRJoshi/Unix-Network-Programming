#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
extern int errno;
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/wait.h>

#include "err_routine.h"
#include "fifo.h"

/*
 * Some disclaimer:
 *    1.  The order of the open system call--used in parent and child process--is significant.
 *        When the parent opens the FIFO1 for writing, it waits until the child opens it for reading.
 *        If the first call to open in the child were for FIFO2 instead of FIFO1, then the child would
 *        wait for the parent to open FIFO2 for writing (the text says this, but i think it should be 
 *        for *reading* as FIFO2 is opened as read file descriptor in parent process). Each process would be
 *        waiting for the other, and neither would proceed. This is called a deadlock.
 *    2.  The parent process is the client (my assumption is that it's easier to set the parent as client as we 
 *        have access to the stdin). And, the child, being the server, need not worry about having to delete 
 *        the FIFOs that were previously created. This is why, after the server is done serving, and the child 
 *        process terminates, it is the client's job to remove the FIFOs that were created.
*/
int main (void) {
  
  int childpid, readfd, writefd;

  /* 
   * int mknod (char *pathname, int mode, int dev);
   * When creating a FIFO (named pipe), the third argument (dev) is ignored for the system call mknod.
   * Also, We need to logically OR the S_IFIFO flag with the file permission to signify a FIFO is being created.
  */
  if ( (mknod(FIFO1, S_IFIFO | PERMS, 0) < 0) && (errno != EEXIST)) {
    err_sys("can't create fifo 1: %s", FIFO1);
  }

  if ( (mknod(FIFO2, S_IFIFO | PERMS, 0) < 0) && (errno != EEXIST)) {
    unlink(FIFO1);        /* unlink/remove the FIFO1 in case FIFO2 cannot be created */
    err_sys("can't create fifo 2: %s", FIFO2);
  }

  if ( (childpid = fork()) < 0) {
    err_sys("can't fork");
  } else if (childpid > 0) {        /* parent process */
    if ( (writefd = open(FIFO1, 1)) < 0) {      /* oflag 1 = O_WRONLY */
      err_sys("parent: can't open write fifo");
    }
    if ( (readfd = open(FIFO2, 0)) < 0) {       /* oflag 0 = O_RDONLY */
      err_sys("parent: can't open read fifo");
    }
    client(readfd, writefd);

    while (wait((int *) 0) != childpid) {
      ;       /* wait till the child process termiates or is terminated. */
    }

    close(writefd);
    close(readfd);

    if (unlink(FIFO1) < 0) {
      err_sys("parent: can't unlink %s", FIFO1);
    }
    if (unlink(FIFO2) < 0) {
      err_sys("parent: can't unlink %s", FIFO2);
    }

    exit(EXIT_SUCCESS);
  } else {                          /* child process */
    if ( (readfd = open(FIFO1, 0)) < 0) {
      err_sys("child: can't open read fifo");
    }
    if ( (writefd = open(FIFO2, 1)) < 0) {
      err_sys("child: can't open write fifo");
    }

    server(readfd, writefd);

    close(readfd);
    close(writefd);

    exit(EXIT_SUCCESS);
  }

  return 0;
}
