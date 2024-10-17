#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
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
 * Functionality: The following things happen in the main function.
 *    ->  The client will first get the shared memory IDs, each having the storage size equivalent to sizeof(Mesg) 
 *    ->  For each shared memory segment created, element of mesgptr--an array of pointers to struct Mesg--is assigned the 
 *        pointer to respective shared memory segment. 
 *    ->  The semaphores are fetched using the sem_open wrapper. sem_open will modify the number of processes that are waiting 
 *        to get hold of the resource.
 *    ->  The client function is called (described below).
 *    ->  After all the message passing is done, its the client process's job to detach and remove the shared memory segments.
 *        shmdt is used to detach, whilst shmctl (with IPC_RMID) is used to remove the shared memory segments.
 *    ->  The client also closes the semaphores. (We are not sure if the its the client or server process which removes the 
 *        semaphore IDs, but the final call to sem_close will remove, given no other processes are waiting.)
 *
 * NOTE:
 *    ->  Because of the nature of SEM_UNDO in my machine, after the server ends up writing the content(s) of the file in the 
 *        shared memory segment, and exits the server process, the SEM_UNDO will "adjust" the semaphore for clisem and servsem.
 *        This introduces "starvation", where the client process will wait for the clisem to be greater than 1, but after the 
 *        adjustment, the value is altered.
 *
 *        To fix this, the solution i found is to remove the SEM_UNDO flag in op_op structure, which checks the semaphore value 
 *        of clisem and servsem (in sem_wait and sem_signal).
 *
 *    ->  Another approach we can use it to test if the clisem is zero (which happens after client writes the last message to 
 *        the standard output) in the server process. This ensures that the client is able to write all the content without 
 *        going into "starvation" because of SEM_UNDO adjustment done from the exit of server process. 
 *
 *        One thing good about this fix is the fact that we don't have to mess around with semaphore.c implementation. This 
 *        solution can be done by the user wriitng the server functionality without dealing with other library.
 *
 *    ->  Not ideal like other solutions, but putting the server process into sleep or usleep before exiting somehow solves
 *        the problem described above.
*/
int main (void) {

  register int    i;

  /*
   * Get the shared memory segments and attach them.
   * We don't specify IPC_CREAT, assuming the server creates them.
  */
  
  for (i = 0; i < NBUFF; i++) {
    if ( (shmid[i] = shmget(SHMKEY + i, sizeof(Mesg), 0)) < 0) {
      err_sys("client: can't get shared memory segment");
    }

    if ( (mesgptr[i] = (Mesg *) shmat(shmid[i], (char *) 0, 0)) == (Mesg *) -1) {
      err_sys("client: can't attach shared memory segment");
    }
  }

  /*
   * Open two semaphores. 
  */
  if ( (clisem = sem_open(SEMKEY1)) < 0) {
    err_sys("client: can't open client semaphore");
  }
  if ( (servsem = sem_open(SEMKEY2)) < 0) {
    err_sys("client: can't open server semaphore");
  }

  client();

  /*
   * Detach and remove the shared memory segments and close the semaphores.
  */
  for (i = 0; i < NBUFF; i++) {
    if (shmdt(mesgptr[i]) < 0) {
      err_sys("client: can't detach shared memory %d", i);
    }
    if (shmctl(shmid[i], IPC_RMID, (struct shmid_ds *) 0) < 0) {
      err_sys("client: can't remove shared memory %d", i);
    }
  }

  /*
    printf("\n[LOG] The value of the client semaphore before closing: %d\n", semctl(clisem, 0, GETVAL));
    printf("\n[LOG] The value of the server semaphore before closing: %d\n", semctl(clisem, 0, GETVAL));
  */

  sem_close(clisem);        /* will remove the semaphore */
  sem_close(servsem);       /* will remove the semaphore */
  
  exit(EXIT_SUCCESS);
}

/*
 * Functionality: The following functionality is done by the client function.
 *    ->  The sem_wait function is used to wait for the client to get access to the shared memory segments.
 *        As the server creates clisem with 1, so the client will have the inital access to memory segment.
 *        NOTE: The index of shared memory segment used by server to read the client data is [0], which is where 
 *        the client writes the name of the file to be read and output to the standard output.
 *    ->  The client, after having access, will first fetch the name of the file from the standard input, and stores
 *        the name of the file in the index [0] of the memory segment pointers.
 *    ->  The server is then sent signal using the sem_signal. This lets the server read the content from 
 *        mesgptr[0], which the shared memory segment where the filename is stored.
 *    ->  Inside the for loop, as we have used the sem_wait on the client before, the client, after encountering the second 
 *        sem_wait, is put to sleep until the server process the file, reads the file content into one of the shared 
 *        memory segment, and signals the client to read the data from the respective memory segment.
 *    ->  We know that the all the data has been retrieved when the server returns the mesgptr[i]->mesg_len of 0, 
 *        and the program jumps to the label `alldone`.
 *        If the server sent value less than 0, then an error has occurred and needs to be handled accordingly.
 *    ->  Coming back the for loop segment, the client waits for the server to send the signal, the client writes the content
 *        from the shared memory segment. and sends the signal to the server, indicating it is ready to write data, if any.
*/
void client (void) {
  int   i, n;
  
  /*
   * Read the filename from standard input, write it to shared memory
  */
  sem_wait(clisem);       /* wait for server to initialize */

  if (fgets(mesgptr[0]->mesg_data, MAXMESGDATA, stdin) == NULL) {
    err_sys("filename read error");
  }

  n = strlen(mesgptr[0]->mesg_data);
  if (mesgptr[0]->mesg_data[n-1] == '\n') {
    n--;      /* ignore new-line from fgets */
  }
  mesgptr[0]->mesg_len = n;
  sem_signal(servsem);                    /* wake up server */

  /*
    printf("[LOG] The value of the client semaphore is: %d\n", semctl(clisem, 0, GETVAL));
  */

  for (;;) {
    for (i = 0; i < NBUFF; i++) {
      sem_wait(clisem);
      if ( (n = mesgptr[i]->mesg_len) <= 0) {
        goto alldone;
      }

      if (write(1, mesgptr[i]->mesg_data, n) != n) {
        err_sys("data write error");
      }
      /*
        printf("\n[LOG-CLIENT] clisem: %d, i: %d, servsem: %d, n: %d\n", semctl(clisem, 0, GETVAL), i, semctl(servsem, 0, GETVAL), n);
      */
      
      sem_signal(servsem);
    }
  }

alldone:
  if (n < 0) {
    err_sys("data read error");
  }
}

