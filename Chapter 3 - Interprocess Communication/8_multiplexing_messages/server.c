#include "mesg.h"
#include "msgq.h"
#include "err_routine.h"

#include <stdlib.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

Mesg mesg;

int main (void) {
  
  int id;

  /*
   * Create the message queue, if required.
  */
  /*
   * IPC_CREAT will create an IPC for the respective key_t variable. 
   * To make a unique IPC channel, IPC_PRIVATE flag is specified.
   * To make sure a new key is made, and no previous key is returned, IPC_EXCL is flag is combined as well.
   * The perms are for the 9-bit mode of the IPC channel.
   * Upon successful call, an message queue id is returned.
  */
  if ( (id = msgget(MKEY1, PERMS | IPC_CREAT)) < 0) {
    err_sys("server: can't get message queue 1");
  }
  
  /*
   * How does the server work?
   *  ->  The server function takes in the message queue id of the IPC channel.
   *  ->  At first, the mesg_type member is set to 1L, signifying that the process
   *      wants to read from the channel of message type 1L. The server reads from this type.
   *  ->  Even though the name of the file--sent from the file--contains a null-termination,
   *      the program probably wants to make sure that the character array is properly 
   *      null-terminated.
   *  ->  After receiving the filename, the process proceeds to change the mesg_type to 2L,
   *      which means the message we are about to send will be of type 2L, which the client 
   *      will read.
   *  ->  If the file cannot be opened, construct an error message and send it to the client.
   *  ->  If the file is read, then read MAXMESGDATA bytes from the file, and then send it 
   *      to the IPC channel. When the file is at EOF, read will return 0, terminating the loop.
   *  ->  An empty envelope (value of mesg_len member = 0) is sent at the end
   *  ->  NOTE: Like stated in the client file, the fuction mesg_send and mesg_recv both are 
   *      wrappers to the system calls msgsnd and msgrcv resepectively.
  */
  server(id);

  exit(EXIT_SUCCESS);
}

void server (msgqid)
int msgqid; {
  int   n, filefd;
  char  errmesg[256], *sys_err_str();

  /*
   * Read the filename message from the IPC descriptor.
  */

  mesg.mesg_type = 1L;                            /* Receive message of this type */
  if ( (n = mesg_recv(msgqid, &mesg)) <= 0) {
    err_sys("server: filename read error");
  }
  mesg.mesg_data[n] = '\0';                       /* null terminate filename */

  mesg.mesg_type = 2L;                            /* send message of this type */
  if ( (filefd = open(mesg.mesg_data, 0)) < 0) {  /* oflag 0 = O_RDONLY */
    /*
     * Error. Format an error message and send it back to the client.
    */
    sprintf(errmesg, ": can't open %s\n", sys_err_str());
    strcat(mesg.mesg_data, errmesg);
    mesg.mesg_len = strlen(mesg.mesg_data);
    mesg_send(msgqid, &mesg);
  } else {
    /*
     * Read the data from the file and send a message to the IPC descriptor.
    */
    while ( (n = read(filefd, mesg.mesg_data, MAXMESGDATA)) > 0) {
      mesg.mesg_len = n;
      mesg_send(msgqid, &mesg);
    }
    close(filefd);

    if (n < 0) {
      err_sys("server: read error");
    }
  }

  /*
   * Send a message with a length of 0 to signify the end.
  */

  mesg.mesg_len = 0;
  mesg_send(msgqid, &mesg);
}
