/*
 * Locking routines using semaphores.
*/

/* includes */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

#include "err_routine.h"

/* defines */
#define   SEMKEY    123456L   /* key value for semget() */
#define   PERMS     0666      /* permissions used for IPC channel mode */

/* structs used for semaphore system calls and other global variables */
static struct sembuf op_lock[2] = { 
                                    {0,0,0},    /* wait for semaphore number 0 to become 0 */ 
                                    {0,1,0}     /* then increment semaphore number 0 by 1 */
                                  };

static struct sembuf op_unlock[1] = {
                                      {0, -1, IPC_NOWAIT}   /* decrement semaphore number 0 by 1 (sets it to 0) */
                                    };

int semid = -1;       /* sempahore id */

void my_lock (fd)
int fd; {
  if (semid < 0) {    /* when semaphore has not been created */
    /* create a semaphore set of 1 sempahore */
    if ( (semid = semget(SEMKEY, 1,  PERMS | IPC_CREAT)) < 0) {
      err_sys("semget error");
    }
  }
  printf("[LOG] The semaphore number is: %d\n", semid);
  /* after the semaphore has been created */
  /*
   * using &op_lock[0] indicates the address to the first element, and 2 specifies the number of semaphore in the semaphore set.
   * now, on understanding the use of the struct sembuf op_lock:
   *  struct sembuf {
   *    ushort  sem_num;    // semaphore number
   *    short   sem_op;     // semaphore operation
   *    short   sem_flg;    // operation flags
   *  }
   *    - The first element contains three members: 0, 0, 0. This indicates that the semaphore number (0) does the operation (0), which means the caller waits till the value of the semaphore is zero, and there is no special flag provided, so the process may wait till the semaphore is avaiable.
   *    - The second element contains three members: 0, 1, 0. This indicates that the sempahore number (0) does the operation (1), which is a positive number, so it indicates the additon of `sem_val` (probably means the member of the `sem` struct) to the semaphore's current value. (NOTE: In my machine, the manual page indicates that a positive value states the increment of semaphore's value by sem_op's amount (which is one (1) in this case). Refer to semop (2) manual page for more info.)
   *    NOTE: The primary idea of the usage of semaphore is to ensure that no race conditions or deadlock conditions arise. To ensure this, the system calls: semget, semop, and semctl must be an atomic operation.
  */
  if (semop(semid, &op_lock[0], 2) < 0) {
    err_sys("semop lock error");
  }
}

void my_unlock (fd)
int fd; {
  /* 
   * Let's try to decode this call as well...
   *    - op_unlock is an array of sturct sembuf which has only one element.
   *    - The element contains the member: {0, -1, IPC_NOWAIT}. What this means is that the semaphore number (0) after doing the operation (-1), which means that the caller will wait for the semaphore value to become greater than or absolute value provided (>= 1 in this case), and it will not wait (IPC_NOWAIT) if the operation can't be completed. Also in the case when the semaphore's value is greater than or equal to the sem_op's absolute value, the sempahore's value is decremented by absolute value of sem_op's specified value (|-1| == 1 in this case.) 
  */
  if (semop(semid, &op_unlock[0], 1) < 0) {
    err_sys("semop unlock error");
  }
}

/*
 * [-] Problem with the implementation:
 *    - If the process aborts for some reason while it has the lock, the semaphore value is left at one. Any other process that tries to obtain the lock waits forever when it does the locking semop that first waits for the value to become zero.
 * [+] Some ways around this:
 *    - A process that has a lock can set up signal handlers to catch all possible signals, and remove the lock before terminating. The only problem with this is that there are some signals that a process can't catch, such as SIGKILL.
 *    - Make `my_lock` more sophisticated. 
 *        Specify the IPC_NOWAIT flag on the first operation in the op_lock array. If the semop returns an error with `errno` equal to `EAGAIN` (Returned when the sempahore's value would result in a process being put to sleep and IPC_NOWAIT is specified.), the process can call the `semctl` system call and look at the sem_ctime value for the semaphore. If some long amount of time has passed since the last change to the semaphore (a few minutes, perhaps) the process can assume that the lock has been abandoned by some other process and can go ahead and remove the lock. (Some issues: We have to use multple system calls every time a file is being locked, and we have to make a guess on what amount of time implies that the lock has been left around.)
 *    - Tell the kernel when we obtain the lock that if this process terminates before releasing the lock, release it for the process.
*/

/*
 * Some footnote: The reason we are using semaphores in "backward" fashion rather than the typical one, where when a resource is allocated, the semaphore is decremented (and zero indicating the holding of resource, and one indicating the resource is freed), is because of the fact that System V semaphore where it is hard to initialize a semaphore to a value other than zero. We observe this in next programs.
*/
