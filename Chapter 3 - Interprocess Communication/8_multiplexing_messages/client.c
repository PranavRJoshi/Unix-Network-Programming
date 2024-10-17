#include "mesg.h"
#include "msgq.h"
#include "err_routine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Mesg mesg;

int main (void) {
  
  int id;

  /*
   * Open the single message queue. The server must have already created it.
  */
  /* 
   * Refer to 7_system_v_ipc/client.c to learn about why the flag of zero is provided.
   * Returns the message queue id, when the call is successful
  */
  if ( (id = msgget(MKEY1, 0)) < 0) {
    err_sys("client: can't msgget message queue 1");
  }

  /*
   * How this call works? 
   *  ->  The client function takes in one argument: the message queue id.
   *  ->  The function attempts to read from the standard input the name of the file whose content is to be viewed.
   *  ->  When sending a message from the client to the server, the mesg_type member is set to 1L (long value of 1).
   *  ->  mesg_send essentially is a wrapper to msgsnd system call, so the message is sent to the queue which 
   *      has the type of 1L (so the server can read the message).
   *  ->  NOTE: The functionality of the server is provided to the respective function definition, for now, we assume 
   *      that the server call is successful, and the content is received.
   *  ->  Notice that the mesg_type member is changed to 2L. This is because the server will send the content of the 
   *      file to the messsage queue of type 2L. 
   *  ->  mesg_recv also is a wrapper to the system call msgrcv. After receiving the file, an attempt is made to write 
   *      the respective content to the standard output.
  */
  client(id);

  /*
   * Now we can delete the message queue.
  */
  /*
   * After the work has been completed, the IPC needs to be closed, which is done through msgctl. 
   * The argument IPC_RMID signifies the closing of the IPC.
  */
  if (msgctl(id, IPC_RMID, (struct msqid_ds *) 0) < 0) {
    err_sys("client: can't RMID message queue 1");
  }

  exit(EXIT_SUCCESS);
}

void client (msgqid)
int msgqid; {
  int n;

  /*
   * Read the filename from standard input, write it as a message to the IPC descriptor.
  */
  /*
   * fgets, according the man page in my machine, appends a new-line character at the end, whilst also 
   * retaining the newline character, if it has found any. 
   *
   * So, when the user provides an input of, say, `client.c`, the strlen function will result in 9.
   * This is because the strlen also accounts the newline as a character in the string.
  */
  if (fgets(mesg.mesg_data, MAXMESGDATA, stdin) == NULL) {
    err_sys("filename read error");
  }

  n = strlen(mesg.mesg_data);
  /*
   * Check if the last character is a newline character.
   * If it is, remove the newline character and replace it with a null character.
   * As the newline is replaced by null termination, we must decrement n by 1 as well.
  */
  if (mesg.mesg_data[n-1] == '\n') {
    n--;                                  /* ignore the newline from fgets() */
  }
  mesg.mesg_data[n] = '\0';               /* overwrite newline (with null termination) at end */
  mesg.mesg_len     = n;
  mesg.mesg_type    = 1L;                 /* send message of this type */
  mesg_send(msgqid, &mesg);

  /*
   * Receive the message from the IPC descriptor and write the data to the standard output.
  */
  mesg.mesg_type = 2L;
  while ( (n = mesg_recv(msgqid, &mesg)) > 0) {
    if (write(1, mesg.mesg_data, n) != n) {
      err_sys("data write error");
    }
  }

  if (n < 0) {
    err_sys("data read error");
  }
}
