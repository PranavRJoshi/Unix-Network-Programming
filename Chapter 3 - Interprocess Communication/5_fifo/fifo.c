#include "fifo.h"           /* for function declaration: client, server */
#include "err_routine.h"    /* for function declaration: err_sys */

#include <stdio.h>
#include <string.h>         /* for function: strlen */
#include <unistd.h>         /* for function: write */
#include <fcntl.h>          /* for function: open */

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
