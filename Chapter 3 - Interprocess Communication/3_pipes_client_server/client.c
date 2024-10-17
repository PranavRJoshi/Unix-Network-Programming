#include "client.h"         /* for function declaration: client */
#include "err_routine.h"    /* for function declaration: err_sys */

#include <stdio.h>
#include <string.h>         /* for function: strlen */
#include <unistd.h>         /* for function: write */

#define MAXBUFF         1024

/*
 * client
 * args:          readfd  - read from the pipe2
 *                writefd - write to the pipe1
 * functionality: Reads from the standard input, stores the content in buff. Reads atmost MAXBUFF characters to the buff.
 *                The read string is then written to the writefd, which is the writing end of pipe1.
 *                After reading the filename, it is sent to the reading end of pipe1, which is read by server function in server.c
 *                While the input from the pipe2 is buffered by server, the client reads it, and sends the buffer to the standard output.
*/
void client (readfd, writefd)
int readfd;
int writefd; {
  char buff[MAXBUFF];
  int n;

  printf("****************CLIENT****************\n");

  /*
   * Read the filename from the standard input, and write it to the IPC descriptor
  */
  if (fgets(buff, MAXBUFF, stdin) == NULL) {
    err_sys("client: filename read error");
  }

  /* printf("[LOG] The file info provided is: %s\n", buff); */

  n = strlen(buff);
  if (buff[n-1] == '\n') {
    n--;                      /* ignore newline from fgets */
  }

  if (write(writefd, buff, n) != n) {
    err_sys("client: filename write error");
  }

  /*
   * Read the data from the IPC descriptor and write to standard output.
  */
  while ( (n = read(readfd, buff, MAXBUFF)) > 0 ) {
    if (write(1, buff, n) != n) {             /* fd 1 = stdout */
      err_sys("client: data write error");
    }
  }

  if (n < 0) {
    err_sys("client: data read error");
  }

  printf("****************CLIENT****************\n");
}
