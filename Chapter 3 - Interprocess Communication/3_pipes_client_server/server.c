#include "server.h"
#include "err_routine.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <string.h>

#define MAXBUFF     1024

/*
 * server
 * args:          readfd  - read from the pipe1
 *                writefd - write to the pipe2
 * functionality: Reads from reading end of pipe1, stores the content in buff. Reads atmost MAXBUFF characters to the buff.
 *                After reading the filename, the file is opened (in read-only) and checked if any error occured. Appropriate 
 *                error message is printed using the sys_err_str--based on the errno. Upon successful call, the fd is now a 
 *                file descriptor to the respective file, and then contents of the file is read at most MAXBUFF and stored to 
 *                the buff array. Recall that read automatically adjusts the offset after each call, so we don't have to manually 
 *                configure the byte offset to the file.
 *                The read item is then buffered to the writefd (writing end of pipe2 which is read by client) in each read call.
 *                After encountering the EOF, the read operation sets the byte-offset to the end of the file such that the next call 
 *                will return a zero. Hence, we know when to terminate reading the file.
 *                The final if statement signifies that read operation was not successful.
*/
void server(readfd, writefd)
int readfd;
int writefd; {
  char        buff[MAXBUFF];
  char        errmesg[256], *sys_err_str(); 
  int         n, fd;
  extern int  errno;

  printf("****************SERVER****************\n");
  /*
   * Read the filename from the IPC descriptor
  */
  if ( (n = read(readfd, buff, MAXBUFF)) <= 0) {
    err_sys("server: filename read error");
  }
  buff[n] = '\0';

  if ( (fd = open(buff, 0)) < 0) {      /* oflag 0 = O_RDONLY */
    /*
     * Error. Format an error message and send it back to the client.
    */
    sprintf(errmesg, ": can't open, %s\n", sys_err_str());  /* attach the corresponding errno status */
    strcat(buff, errmesg);      /* buff contains the filename read from pipe1 (written by client) */
    n = strlen(buff);

    if (write(writefd, buff, n) != n) {
      err_sys("server: errmesg write error");
    }
  } else {
    /*
     * Read the data from the file and write to the IPC descriptor.
    */
    while ( (n = read(fd, buff, MAXBUFF)) > 0) {
      if (write(writefd, buff, n) != n) {
        err_sys("server: data write error");
      }
    }
    if (n < 0) {
      err_sys("server: read error");
    }
  }
  printf("****************SERVER****************\n");
}
