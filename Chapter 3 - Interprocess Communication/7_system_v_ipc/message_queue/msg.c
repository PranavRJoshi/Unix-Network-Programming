#include <sys/types.h>
#include <sys/ipc.h>      /* optional as the msg.h header also includes this header */
#include <sys/msg.h>
#include <stdio.h>
#include <limits.h>

#include "err_routine.h"

#define   KEY     ((key_t) 98765L)
#define   PERMS   0666

/* 
 * Bummer that my system does not have a manual page for the system calls like msgget and msgctl.
 * Also, the text speicifies that "if the system is configured for a maximum of 50 message queues..."
 * Seems like the maximum of 65536 message queues is present in my machine.
*/
int main (void) {
  
  int i, msqid;

  printf("The maximum value of int is: %d\n", INT_MAX);

  for (i = 0; i < 10; i++) {
    /* Take the key as the first argument, set the mode as the second argument. It is one of the get system call */
    if ( (msqid = msgget(KEY, PERMS | IPC_CREAT)) < 0) {
      err_sys("can't create message queue and the returned value is: %d", msqid);
    }

    printf("msqid = %d\n", msqid);

    /* One of the control system call for the message queue. */
    if (msgctl(msqid, IPC_RMID, (struct msqid_ds *) 0) < 0) {
      err_sys("can't remove message queue");
    }
  }

  return 0;
}
