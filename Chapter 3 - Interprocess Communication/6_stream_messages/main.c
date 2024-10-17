#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
extern int errno;
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/wait.h>

#include "err_routine.h"
#include "ipc.h"

/*
 * Functionality: creates a parent and a child process using fork (after creating two FIFOs), the parent process
 *                acts as client and the child process acts as a server. The end user writes the filename to the 
 *                client and it is processed into a structure of type Mesg (4096 bytes). The structure contains 
 *                the message length, the type of message, and the actual message content in byte (char) array.
 *                The parent uses FIFO1 to write the message to the server. After writing the filename in the 
 *                reading end of FIFO2 (parent has access to), the server opens up the provided file (if it 
 *                exists) and reads the content of the file. The read content is stored as a message envelope
 *                which is sent to FIFO2 by the server. The client reads it and writes it into the standarad 
 *                output. 
*/
int main (void) {
  
  int childpid, readfd, writefd;

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

