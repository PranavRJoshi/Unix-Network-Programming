#include "msgq.h"
#include "err_routine.h"
#include "mesg.h"

#include <stdlib.h>

int main (void) {

  int   readid, writefd;

  /*
   * Open the message queues.
   * The server must have already created them.
  */
  /*
   * Reference: https://www.ibm.com/docs/en/zos/2.2.0?topic=functions-msgget-get-message-queue
   *
   * REMINDER:  The function declaration of the system call msgget is:
   *
   *                int msgget (key_t key, int msgflag);
   *
   *            The msgflag value is a combination of the constants:
   *            
   *            |---------|------------|------------------|
   *            | Numeric | Symbolic   |    Description   |
   *            |---------|------------|------------------|
   *            | 0400    | MSG_R      |  Read by owner   |
   *            | 0200    | MSG_W      |  Write by owner  |
   *            | 0040    | MSG_R >> 3 |  Read by group   |
   *            | 0020    | MSG_W >> 3 |  Write by group  |
   *            | 0004    | MSG_R >> 6 |  Read by world   |
   *            | 0002    | MSG_W >> 6 |  Write by world  |
   *            |         | IPC_CREAT  |  See Section 3.8 |
   *            |         | IPC_EXCL   |  See Section 3.8 |
   *            |---------|------------|------------------|
   *
   *            The value returned by msgget is the message queue identifier, msqid, or -1 if an error occured.
   * 
   * The meaning of having the msgflag as 0 indicates the following:
   *    1. If a message queue identifier has already been created with key earlier, and the calling process of this msgget() has read and/or write permissions to it, then msgget() returns the associated message queue identifier.
   *    2. If a message queue identifier has already been created with key earlier, and the calling process of this msgget() does not have read and/or write permissions to it, then msgget() returns -1 and sets errno to EACCES.
   *    3. If a message queue identifier has not been created with key earlier, thne msgget() returns -1 and sets errno to ENOENT.
  */
  if ( (writefd = msgget(MKEY1, 0)) < 0) {
    err_sys("client: can't msgget message queue 1");
  }
  if ( (readid = msgget(MKEY2, 0)) < 0) {
    err_sys("client: can't msgget message queue 2");
  }

  client(readid, writefd);

  /*
   * Now we can delete the message queue.
  */
  if (msgctl(readid, IPC_RMID, (struct msqid_ds *) 0) < 0) {
    err_sys("client: can't RMID message queue 1");
  }
  if (msgctl(writefd, IPC_RMID, (struct msqid_ds *) 0) < 0) {
    err_sys("client: can't RMID message queue 2");
  }

  exit(EXIT_SUCCESS);
}
