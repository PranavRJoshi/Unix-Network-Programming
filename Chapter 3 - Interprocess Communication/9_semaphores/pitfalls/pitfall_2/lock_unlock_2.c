/*
 * Locking routines using semaphores.
 * Use the SEM_UNDO feature to have the kernel adjust the semaphore value on premature exit.
*/

/* Some enhancements over the previous program:
 *    - When a semaphore value is initialized, whether created by `semget`, or specifically set by `semctl`'s SETVAL or SETALL commands, the adjustment value for the particular semaphore value is set to zero. Similarly, when a semaphore is deleted, any adjustment values associated with it are also deleted.
 *    - For every `semop` operation that specifies the SEM_UNDO flag, if the semaphore value goes up, the adjustment value goes down by the same amount. If the semaphore value goes down, the adjustment value goes up by the same amount.
 *    - When a process `exit`s, the kernel automatically applies any adjustment values for that process.
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "err_routine.h"

#define   SEMKEY    123456L   /* key value for semget() */
#define   PERMS     0666      /* IPC Channel mode permission */

static struct sembuf op_lock[2] = {
                                    {0, 0, 0},          /* wait for semaphore number 0 to become 0 */
                                    {0, 1, SEM_UNDO}    /* then increment semaphore number 0 by 1 */
                                  };

static struct sembuf op_unlock[1] = {
                                      {0, -1, (IPC_NOWAIT | SEM_UNDO)}  /* decrement semaphore number 0 by 1 (sets it to 0) */
                                    };

int semid = -1;   /* semaphore id */

void my_lock (fd)
int fd; {
  if (semid < 0) {
    /* 
     * As there is no semaphore, get one using the semget system call.
     * Using the call, get a semaphore set with one sempahore, and create it using IPC_CREAT and mode of IPC file of PERMS.
     * The key of the semaphore is SEMKEY
    */
    if ( (semid = semget(SEMKEY, 1, IPC_CREAT | PERMS)) < 0) {
      err_sys("semget error");
    }
  }

  /*
   * The semop system call operates on the struct sembuf (op_lock) which has two elements as indicated by the third argument (2).
   * The semaphore to work on is indicated by semid.
   * The operations specified are:
   *    - For the first element: For the sem_num of zero (0), the sem_op is zero (0) which indicates that the calling process wait for the value of semaphore to become zero, and since no flag (like IPC_NOWAIT) is specified, the caller is put into sleep.
   *    - To be more verbose, when creating a semaphore set using the semget system call, we specified that the number of elements in the semaphore set be one (1). So, the reason we only have the first members of the struct sembuf `op_{lock|unlock}` have only 0, is that it works on the first element of the semaphore set. For instance, if we have initialized semaphore set with more than one elements, and to access the other semaphores, we'd set up the sembuf struct such as: {1/2/3..., operation_num, some_flags}, where 1 indicates the second element of the semaphore set, and so on.
  */
  if (semop(semid, &op_lock[0], 2) < 0) {
    err_sys("semop lock error");
  }
}

void my_unlock (fd)
int fd; {
  if (semop(semid, &op_unlock[0], 1) < 0) {
    err_sys("semop unlock error");
  }
}

/*
 * Still some problems...
 *    - The semaphore is never removed from the system. The main function that calls our locking functions should call the `semctl` system call with command argument of IPC_RMID before exiting, if it is the last process using the semaphore.
 *    - The SEM_UNDO feature described eariler only assures that the actual semaphore value gets adjusted as required if the process exits prematurely. It does not remove a semaphore that is not used by any active process.
*/
