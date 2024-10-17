#include "msgq.h"
#include "err_routine.h"
#include "mesg.h"

#include <stdlib.h>
#include <sys/msg.h>

int main (void) {
  
  int   readid, writeid;

  /*
   * Create the message queues, if required.
  */
  /*
   * NOTE:  Refer to client.c to learn about the system call msgget
   *        The man page for this system call, along with other such as 
   *          -> msgsnd
   *          -> msgrecv, and 
   *          -> msgctl
   *        are not provided in my machine
  */
  if ( (readid = msgget(MKEY1, PERMS | IPC_CREAT)) < 0) {
    err_sys("server: can't get message queue 1");
  }
  if ( (writeid = msgget(MKEY2, PERMS | IPC_CREAT)) < 0) {
    err_sys("server: can't get message queue 2");
  }

  server(readid, writeid);

  exit(EXIT_SUCCESS);
}
