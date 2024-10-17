#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "mesg.h"
#include "shm.h"
#include "err_routine.h"
#include "semaphore.h"

int   shmid, clisem, servsem;   /* shared memory and semaphore IDs */
Mesg  *mesgptr;                 /* pointer to message structure, which is in the shared memory segment */

int main (void) {

  /*
   * Create the shared memory segment, if required, then attach it.
  */
  if ( (shmid = shmget(SHMKEY, sizeof(Mesg), IPC_CREAT | PERMS)) < 0) {
    err_sys("server: can't get shared memory");
  }

  /* 
   * make mesgptr point to the shared memory space which is defined by the shmid got through shmget
   * mesgptr, if unsuccessful, returns -1 and sets errno (check man page for shmat (2))
  */
  if ( (mesgptr = (Mesg *) shmat(shmid, (char *) 0, 0)) == (Mesg *) -1) {
    err_sys("server: can't attach shared memory");
  }

  /*
   * Create two semaphores. The client semaphore starts out at 1 since the client process starts thing going.
   *  
   * The client first sends a message to the server querying about the filename to be printed in standard output.
   * The server reads from the shared memory, and then opens the file (if it exists). The content of the file is then
   * written to the shared memory, which the client is given access to shortly. The client then reads the content, 
   * writes it to the standard output, provides shared memory access to the server, which is repeated until the server
   * reads all the file content. The server, after finishing, will send an empty message, and closes the semaphores.
   * Also, the shared memory is detached from the server, and closed by the client. The client will also close its 
   * defined semaphores. 
  */

  if ( (clisem = sem_create(SEMKEY1, 1)) < 0) {
    err_sys("server: can't create client semaphore");
  }
  if ( (servsem = sem_create(SEMKEY2, 0)) < 0) {
    err_sys("server: can't create server semaphore");
  }

  server();

  /*
   * Detach the shared memory segment and close the semaphores.
   * The client is the last one to use the shared memory, so it'll remove it when it's done.
  */

  if (shmdt(mesgptr) < 0) {
    err_sys("server: can't detach shared memory");
  }

  /*
    printf("[LOG] The sempahore value of the server is: %d\n", semctl(servsem, 1, GETVAL));
    printf("[LOG] The sempahore value of the client is: %d\n", semctl(clisem, 1, GETVAL));
  */

  sem_close(clisem);
  sem_close(servsem);

  exit(EXIT_SUCCESS);
}

void server (void) {
  int   n, filefd;
  char  errmesg[256], *sys_err_str();

  /*
   * wait for the client to write the filename into shared memory.
  */
  sem_wait(servsem);        /* we'll wait here for client to start things */

  mesgptr->mesg_data[mesgptr->mesg_len] = '\0';   /* null terminate filename */

  if ( (filefd = open(mesgptr->mesg_data, 0)) < 0) {    /* file cannot be opened */
    /*
     * Error. Format an error message and send it back to the client.
    */
    sprintf(errmesg, ": can't open, %s", sys_err_str());
    strcat(mesgptr->mesg_data, errmesg);
    mesgptr->mesg_len = strlen(mesgptr->mesg_data);
    sem_signal(clisem);         /* send to client */  /* shared memory access given to client */
    sem_wait(servsem);          /* wait for client to process */  /* sleep till the client uses sem_signal(servsem) */
  } else {  /* file is opened */
    /*
     * Read the data from the file right into shared memory. 
     * The -1 (MAXMESGDATA - 1) in the number-of-bytes-to-read is because some Unices have a bug if you 
     * and read into the final byte of a shared memory segment.
    */
    while ( (n = read(filefd, mesgptr->mesg_data, MAXMESGDATA - 1)) > 0) {
      mesgptr->mesg_len = n;
      sem_signal(clisem);     /* send to client */
      sem_wait(servsem);      /* wait for client to process */  /* sleep till the client signals the servsem */
    }
    close(filefd);

    if (n < 0) {
      err_sys("server: read error");
    }
  }

  /*
   * Send a message with a length of 0 to signify the end.
  */

  mesgptr->mesg_len = 0;
  sem_signal(clisem);
}
