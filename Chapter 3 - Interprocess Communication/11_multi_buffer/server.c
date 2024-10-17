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

int   clisem, servsem;        /* semaphore IDs */
int   shmid[NBUFF];           /* shared memory IDs */
Mesg  *mesgptr[NBUFF];        /* pointer to message structure, which are in the shared memory segment */

/*
 * Functionality: The following functionality is provided by the main process of server.
 *    ->  First, the shared memory segment is initialzed (IPC_CREAT) using the appropriate PERMS.
 *        The shared memory segment is fetched using the shmat and assigned to each indices of 
 *        mesgptr array.
 *    ->  The client (clisem) and server (servsem) semaphores are also initialized. 
 *        The server is responsible for initializing the semaphores using the sem_create.
 *    ->  The server function is called (described below).
 *    ->  After the work is done, the memory segments are detached. It is the client's job to remove the 
 *        shared memory segments.
 *    ->  Lastly, the semaphores are closed (We are not sure which process is the one which removes it from 
 *        the system, but it is done by sem_close by itself).
*/
int main (void) {

  register int      i;

  /*
   * Get the shared memory segments and attach them
  */
  for (i = 0; i < NBUFF; i++) {
    if ( (shmid[i] = shmget(SHMKEY + i, sizeof(Mesg), PERMS | IPC_CREAT)) < 0) {
      err_sys("server: can't get shared memory");
    }

    if ( (mesgptr[i] = (Mesg *) shmat(shmid[i], (char *) 0, 0)) == (Mesg *) -1) {
      err_sys("server: can't attach shared memory");
    }
  }

  /*
   * Create two semaphores. The client semaphore starts out at 1 since the client process starts thing going.
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
  for (i = 0; i < NBUFF; i++) {
    if (shmdt(mesgptr[i]) < 0) {
      err_sys("server: can't detach shared memory");
    }
  }

  /*
    printf("[LOG] The client semaphore value before exiting: %d\n", semctl(clisem, 0, GETVAL));
    printf("[LOG] The server semaphore value before exiting: %d\n", semctl(servsem, 0, GETVAL));
  */
  
  while (semctl(clisem, 0, GETVAL) != 0) {
    ;
  }

  sem_close(clisem);
  sem_close(servsem);

  return 0;
}

/*
 * Functionality: The following works are performed by the server function:
 *    ->  The server first waits for the client to send the signal to the server.
 *        This is done after the client is done writing the name of the file to be read in the 
 *        shared memory segment--which is in the index 0 of mesgptr.
 *    ->  After the filename is fetched, the server then reads the name of the file and attempts to 
 *        open it.
 *        In case the file cannot be opened, the server writes the error message in the 0th index of 
 *        mesgptr and signals the client (to read the error message). The server then waits for the 
 *        client to send a signal (as it is not aware that an error has occured, so we send a length 
 *        of zero in the 1st index of the mesgptr to indicate the end of reading and the client will 
 *        terminate.)
 *    ->  In the case where the server is able to open up the file, the function will first set up 
 *        the possible signals (using sem_signal), the first for loop which has sem_signal(servsem).
 *    ->  Now, the for loop will then use sem_wait for the server to read the contents of the file, 
 *        and writes it the mesgptr[i], where each index is filled with data.
 *        After all the data has been read, the read will return zero, indicating that the file has
 *        been completely read.
*/
void server (void) {
  register int    i, n, filefd;
  char            errmesg[256], *sys_err_str();

  /*
   * wait for the client to write the filename into shared memory.
  */
  sem_wait(servsem);        /* we'll wait here for client to start things */

  mesgptr[0]->mesg_data[mesgptr[0]->mesg_len] = '\0';   /* null terminate filename */

  if ( (filefd = open(mesgptr[0]->mesg_data, 0)) < 0) {    /* file cannot be opened */
    /*
     * Error. Format an error message and send it back to the client.
    */
    sprintf(errmesg, ": can't open, %s", sys_err_str());
    strcat(mesgptr[0]->mesg_data, errmesg);
    mesgptr[0]->mesg_len = strlen(mesgptr[0]->mesg_data);
    sem_signal(clisem);         /* send to client */  /* shared memory access given to client */

    sem_wait(servsem);          /* wait for client to process */  /* sleep till the client uses sem_signal(servsem) */
    mesgptr[1]->mesg_len = 0;
    sem_signal(clisem);         /* wake up client */

  } else {  /* file is opened */
    /*
     * Initialize the server semaphore to the number of buffers.
     * We know its value is 0 now, since it was initialized to 0, and the client has done a 
     * sem_signal(), followed by our sem_wait() above. 
     * What we do is increment the semaphore value once for every buffer (i.e., the number of resources we have).
    */
    for (i = 0; i < NBUFF; i++) {
      sem_signal(servsem);
    }

    /*
      printf("[LOG] The value of the server semaphore is: %d\n", semctl(servsem, 0, GETVAL));
    */

    /*
     * Read the data from the file right into shared memory.
     * The -1 in the number-of-bytes-to-read is because some Unices have a bug if you try and read into the 
     * final byte of a shared memory segment.
    */
    for (;;) {
      for (i= 0; i < NBUFF; i++) {
        sem_wait(servsem);
        /*
          printf("[LOG] The value of the server semaphore is: %d\n", semctl(servsem, 0, GETVAL));
          printf("[LOG] The value of the client semaphore is: %d\n", semctl(clisem, 0, GETVAL));
        */
        n = read(filefd, mesgptr[i]->mesg_data, MAXMESGDATA - 1);
        if (n < 0) {
          err_sys("server: read error");
        }
        mesgptr[i]->mesg_len = n;
        sem_signal(clisem);
        if (n == 0) {
          goto alldone;
        }
      }
    }

  alldone:
    /* we've already written the 0-length final buffer */
    close(filefd);
  }
}
