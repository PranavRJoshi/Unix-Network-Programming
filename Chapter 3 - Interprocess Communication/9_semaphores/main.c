#include "err_routine.h"
#include "semaphore.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define   SEQFILE   "seqno"
#define   SEMKEY    ((key_t) 23456L)
#define   MAXBUFF   100

int main (void) {
  
  int   fd, i, n, pid, seqno, semid;
  char  buff[MAXBUFF];

  pid = getpid();

  if ( (fd = open(SEQFILE, 2)) < 0) {
    err_sys("can't open %s", SEQFILE);
  }

  if ( (semid = sem_create(SEMKEY, 1)) < 0) {
    err_sys("can't open semaphore");
  }

  for (i = 0; i < 20; i++) {
    sem_wait(semid);      /* get the lock */

    lseek(fd, 0L, 0);     /* rewind before read */

    if ( (n = read(fd, buff, MAXBUFF)) <= 0) {
      err_sys("read error");
    }
    buff[n] = '\0';       /* null terminate for sscanf */

    if ( (n = sscanf(buff, "%d", &seqno)) != 1) {
      err_sys("sscanf error");
    }

    printf("pid = %d, seq number = %d\n", pid, seqno);

    seqno++;

    sprintf(buff, "%03d\n", seqno);

    n = strlen(buff);

    lseek(fd, 0L, 0);     /* rewind, again */

    if (write(fd, buff, n) != n) {
      err_sys("write error");
    }

    sem_signal(semid);
  }

  sem_close(semid);       /* release the lock */

  exit(EXIT_SUCCESS);
}
