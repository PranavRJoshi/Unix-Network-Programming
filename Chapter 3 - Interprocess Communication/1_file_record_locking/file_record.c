#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "lock.h"
#include "err_routine.h"

#define SEQFILE "seqno"
#define MAXBUFF 100

int main (void) {
  
  int   fd, i, n, pid, seqno;
  char  buff[MAXBUFF + 1];

  /*
   * get the process id.
  */
  pid = getpid();

  if ( (fd = open(SEQFILE, O_RDWR)) < 0) {
    err_sys("can't open %s\n", SEQFILE);
  }

  /* printf("[LOG] Process ID of %d has the file descriptor of %d\n", getpid(), fd); */

  for (i = 0; i < 20; i++) {
    my_open_lock(fd);                /* lock the file */

    lseek(fd, 0L, 0);                 /* rewind before read */

    /* read from file pointed to by fd, and store the content in buff. read atmost MAXBUFF characters long. */
    if ( (n = read(fd, buff, MAXBUFF)) <= 0) {
      err_sys("read error\n");
    }
    buff[n] = '\0';                   /* null terminate for sscanf */

    /* reads the value from buf--of sequence: a number and a new-line--and stores the number in seqno */
    if ( (n = sscanf(buff, "%d\n", &seqno)) != 1) {
      err_sys("sscanf error\n");
    }

    printf("pid = %d, seq# = %d\n", pid, seqno);

    seqno++;                          /* increment the sequence number */

    sprintf(buff, "%03d\n", seqno);   /* Write to the string buff with width of 3, if the digits are less than 3, zero-padding is done */

    n = strlen(buff);                 /* get the length of the buffer, should be 3 */
    lseek(fd, 0L, 0);                 /* rewind before write */

    if (write(fd, buff, n) != n) {
      err_sys("write error");
    }

    my_open_unlock(fd);                    /* unlock the file */
  }

  return 0;
}
