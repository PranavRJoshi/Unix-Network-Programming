#include "ipc.h"
#include "msg.h"
#include "err_routine.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

Mesg  mesg;

/*
 * Functionality: When the function is called, it searches in standard input for (MAXMESGDATA -1) bytes and stores it in 
 *                mesg_data member of struct tag mesg. fgets stops when a newline character is found, at end-of-file (EOF) 
 *                or error. newline is retained (in my machine) and if there is no character before an EOF, the function
 *                fgets returns NULL. The newline is removed and the length of the input read is stored in mesg_len member
 *                (mesg_type acts like dummy for now). The mesg structure is sent as a pointer to the mesg_send function 
 *                (defined in msg.c). Next, message is awaited from the mesg_recv function and whenever there is message,
 *                it is written in the standard output.
 *
 *                NOTE: The while loop checks for the mesg_recv function to return a value greater than 0. Usually, this is 
 *                because a value of zero indicates that the function received an empty envelope (sent by the server at the 
 *                end). A negative value indicates the inner system call failed.
*/
void client (ipcreadfd, ipcwritefd)
int ipcreadfd;
int ipcwritefd; {

  int n;

  /*
   * Read the filename from standard input, write it as a 
   * message to the IPC descriptor.
  */
  if (fgets(mesg.mesg_data, MAXMESGDATA, stdin) == NULL) {
    err_sys("filename read error");
  }

  n = strlen(mesg.mesg_data);
  if (mesg.mesg_data[n-1] == '\n') {
    n--;      /* ignore newline from fgets() */
  }

  mesg.mesg_len   = n;
  mesg.mesg_type  = 1L;
  mesg_send(ipcwritefd, &mesg);

  /*
   * Receive the message from the IPC descriptor and write the data to the standard output.
  */
  while ( (n = mesg_recv(ipcreadfd, &mesg)) > 0) {
    if (write(1, mesg.mesg_data, n) != n) {
      err_sys("data write error");
    }
  }

  if (n < 0) {
    err_sys("data read error");
  }
}

