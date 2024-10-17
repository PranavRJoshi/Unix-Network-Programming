#include "semaphore.h"
#include "err_routine.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*
 * We create and use a 3-member set for the requested semaphore.
 * Recall that the function semget() requires the caller to enter the number of semaphores (nsems) in the set.
 *
 * -  The first member, [0], is the actual semaphore value
 * -  The second member, [1], is a counter used to know when all processes have finished with the semaphore.
 * -  The third member, [2], of the semaphore set is used as a lock variable to avoid any race conditions in the 
 *    sem_create() and sem_close() functions.
 *
 * [1]  Regarding the second member, [1], the counter is initialized to a large number, decremented on every create or 
 *      open and incremented on every close. 
 * [1]  This way, we can use the "adjust" feature provided by system V so that any process that exit's without calling 
 *      sem_close() is accounted for.
 * [1]  It doesn't help us if the last process does this (as we have no way of getting control to remove the semaphore)
 *      but it will work if any process other than the last does an exit (intentional or unintentional).
*/

#define   BIGCOUNT    10000       /* intial value of process counter. */
#define   PERMS       0666        /* IPC access mode flags. */

/*
 * Define the semaphore operation array for the semop() calls.
*/

static struct sembuf op_lock[2] = {
                                    {2, 0, 0},          /* wait for [2] (lock) to equal 0. */
                                    {2, 1, SEM_UNDO}    /* then increment [2] to 1 -- this locks it. */
                                  };                    /* UNDO to release the lock if processes exits before explicitly unlocking. */

static struct sembuf op_endcreate[2] =  {
                                          {1, -1, SEM_UNDO},  /* decrement [1] (proc counter) with undo on exit. */
                                          {2, -1, SEM_UNDO}   /* UNDO to adjust proc counter if process exits before explicitly calling
                                                                 sem_close(). */
                                        };                    /* then decrement [2] (lock) back to 0. */

static struct sembuf op_open[1] = {
                                    {1, -1, SEM_UNDO} /* decrement [1] (proc counter) with undo on exit. */
                                  };

static struct sembuf op_close[3] =  {
                                      {2, 0, 0},        /* wait for [2] (lock) to equal 0 */
                                      {2, 1, SEM_UNDO}, /* then increment [2] to 1 - this locks it */
                                      {1, 1, SEM_UNDO}  /* then increment [1] proc counter */
                                    };

static struct sembuf op_unlock[1] = {
                                      {2, -1, SEM_UNDO} /* decrement [2] (lock) back to 0. */
                                    };

static struct sembuf op_op[1] = {
  #ifdef __APPLE__
                                  /* {0, 99, 0} */  /* check the notes on client.c */
                                  {0, 99, SEM_UNDO} /* decrement or increment [0] with undo on exit. */
  #else 
                                  {0, 99, SEM_UNDO}
  #endif
                                };                  /* the 99 is set to the actual amount to add or subtract (positive or negative). */

/* Function definitions which are declared in semaphore.h header */

/*
 * sem_create:  Creates a semaphore (if one doesn't exist). As IPC_CREAT is not OR'd with IPC_EXCL, there is no error if the 
 *              process isn't the one that created the channel and the existing semaphore id is used. Upon first call with 
 *              the provided `key`, the proc_counter [1] and actual semaphore [0] is set to BIGCOUNT and `initval` respectively.
 *              Rest of the call will decrement proc_counter [1] by 1.
*/
int sem_create (key, initval)
key_t key;      /* initval: used if we create the semaphore */
int   initval; {
  register int  id, semval;
  union semun {
    int             val;
    struct semid_ds *buf;
    ushort          *array;
  } semctl_arg;

  if (key == IPC_PRIVATE) {
    return -1;    /* not intended for private semaphores. */
  } else if (key == (key_t) -1) {
    return -1;    /* probably an ftok() error by caller. */
  }

again:
  /* Create a semaphore set of 3 members using the key provided. We assume the key hasn't been created. */
  if ( (id = semget(key, 3, PERMS | IPC_CREAT)) < 0) {
    return -1;    /* permission problem or tables full */
  }

  /*
   * NOTE that id holds the semaphore ID (semid) of the semaphore set returned by the kernel.
  */

  /*
   * When the semaphore is created, we know that the value of all 3 members is 0.
   * Get a lock on the semaphore by waiting for [2] to equal to zero (0), then increment it.
   *
   * NOTE:  There is a race condition here.
   *        - There is a possibility that between the semget() and the semop() below, another process can call 
   *          our sem_close() function which can remove the semaphore if that process is the last one using it.
   *        - Therefore, we can handle the error condition of an invalid semaphore ID specially below, and if 
   *          it does happen, we just go back and create it again.
  */
  if (semop(id, &op_lock[0], 2) < 0) {
    if (errno == EINVAL) {
      goto again;
    }
    err_sys("can't lock");
  }
  
  /*
   * Get the value of the process counter. If it equals to zero (0), then no one has initialized the semaphore yet.
  */
  /*
   * What this does is, it gets the second member of the semaphore set [1] (second argument) from the `semid` of `id`
   * and the GETVAL fetches the current semaphore value. The semun argument is not used for GETVAL, so 0 is passed.
  */
  if ( (semval = semctl(id, 1, GETVAL, 0)) < 0) {
    err_sys("can't GETVAL");
  }

  if (semval == 0) {
    /*
     * -  We could initialize by doing an SETALL, but that would clear the adjust value that we set when we locked
     *    the semaphore above.
     * -  Instead, we'll do system calls to initialize [0] and [1].
    */
    /* for [0] member of the semaphore set. (semaphore value) */
    semctl_arg.val = initval;
    /*
     * Here, semctl is used for the SETVAL, so we need to provide the semaphore value which is given through 
     * the semctl_arg.val member. Other members are not required (check the notes)
    */
    if (semctl(id, 0, SETVAL, semctl_arg) < 0) {
      err_sys("can't SETVAL[0]");
    }

    /* for [1] member of the semaphore set. (process counter) */
    semctl_arg.val = BIGCOUNT;
    if (semctl(id, 1, SETVAL, semctl_arg) < 0) {
      err_sys("can't SETVAL[1]");
    }
  }

  /*
   * Decrement the process counter and then release the lock.
  */
  if (semop(id, &op_endcreate[0], 2) < 0) {
    err_sys("can't end create");
  }

  /* return the semaphore id which hold a semaphore set of 3 members. */
  return id;
}

/*
 * sem_open:  Fetch the semaphore id using the key, and then decrement the proc counter [1] by 1 unless [1] < 1 (kernel adjusted).
 *            If the proc counter is < 1 (unlikely as [1] is set to 10000 (BIGCOUNT) at creation), then wait for the [1] to be 
 *            >= 1. and then decrement it.
 *            Returns the semaphore id. (cannot be IPC_PRIVATE or (key_t) -1)
*/
int sem_open (key)
key_t key; {
  register int  id;

  if (key == IPC_PRIVATE) {
    return -1;    /* not intended for private semaphores. */
  } else if (key == (key_t) -1) {
    return -1;    /* probably an ftok() error by caller. */
  }

  /* 
   * This call to semget is used to fetch the semaphore set using the key provided. 
   * Since we are not interested in creating the set, but rather use the existing set,
   * we will specify the `semflag` argument as 0, to indicate this.
  */
  if ( (id = semget(key, 3, 0)) < 0) {
    return -1;    /* doesn't exist, or tables full */
  }

  /*
   * Decrement the process counter. 
   * We don't need a lock to do this.
  */
  if (semop(id, &op_open[0], 1) < 0) {
    err_sys("can't open");
  }

  return id;
}

/*
 * sem_rm:  Remove the semaphore id using the IPC_RMID flag.
*/
void sem_rm (id)
int id; {
  /*
   * From the semaphore set `id`, remove it.
   * Second argument (semnum) and the last argument (union semun) are insignificant in this case.
  */
  if (semctl(id, 0, IPC_RMID, 0) < 0) {
    err_sys("can't IPC_RMID");
  }
}

/*
 * sem_close: wait for the lock [2] to be zero, then increment it (kernel adjusted), and increment proc counter [1] by 1 (kernel
 *            adjusted).
 *            If there are no other processes waiting, remove the semaphore id.
 *            Else wait for lock [2] to be >= 1, and then decrement it by 1 (kernel adjusted).
*/
void sem_close (id)
int id; {
  register int  semval;

  /*
   * The following semop() first gets a lock on the semaphore, then increments [1]--the process counter.
  */
  if (semop(id, &op_close[0], 3) < 0) {
    err_sys("can't semop: %s", strerror(errno));
    /* err_sys("can't semop"); */
  }

  /*
   * Now that we have a lock, read the value of the process counter to see if this is the last reference to the semaphore.
   * There is a race condition here--see the comments in sem_create().
  */
  if ( (semval = semctl(id, 1, GETVAL, 0)) < 0) {
    err_sys("can't GETVAL");
  }

  if (semval > BIGCOUNT) {
    err_dump("sem[1] > BIGCOUNT");
  } else if (semval == BIGCOUNT) {
    sem_rm(id);
  } else {
    if (semop(id, &op_unlock[0], 1) < 0) {
      err_sys("can't unlock");    /* unlock */
    }
  }
}

/*
 * sem_wait:  wait for the [0] member to become >= 1 and decrements, or the caller is put to sleep until the 
 *            condition is satisfied.
*/
void sem_wait (id)
int id; {
  sem_op(id, -1);
}

/*
 * sem_signal:  increments the [0] member by 1 (kernel adjusted).
*/
void sem_signal (id)
int id; {
  sem_op(id, 1);
}

/*
 * sem_op:  if the value is negative, wait till the semaphore value of [0] member is >= |value| and then decrement it by |value|,
 *          else the semaphore value of [0] member is incremented by value. Cannot be zero.
*/
void sem_op (id, value)
int id;
int value; {
  if ( (op_op[0].sem_op = value) == 0) {
    err_sys("can't have value == 0");
  }

  if (semop(id, &op_op[0], 1) < 0) {
    err_sys("sem_op error");
  }
}
