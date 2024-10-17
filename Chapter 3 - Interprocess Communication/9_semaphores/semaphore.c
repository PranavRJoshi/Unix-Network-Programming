#include "semaphore.h"
#include "err_routine.h"

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
                                  {0, 99, SEM_UNDO} /* decrement or increment [0] with undo on exit. */
                                };                  /* the 99 is set to the actual amount to add or subtract (positive or negative). */

/* Function definitions which are declared in semaphore.h header */

/*
 * Before delving into the function definitions hidden within the documentation, lets try to summarize which sembuf structs
 * are utilized by which functions.
 *
 * When refering to the "value" for it's increment and decrement, we're refering to the semaphore value of the semaphore member
 * in the semaphore set (too many semaphores...)
 *
 *    Function      struct sembuf       Description
 *    ------------|-------------------|-------------------------------------------------------------------------------------------------
 *    sem_create    op_lock             Works on the lock semaphore [2] to make it equal to zero first (blocking).
 *                                      Also works on the lock semaphore [2] to increment the semaphore value by 1. (adjustment)
 *
 *                  op_endcreate        Works on the proc counter semaphore [1] to wait and then decrement the value by 1. (adjustment)
 *                                      Also works on the lock semaphore [2] to wait and decrement the value by 1. (adjustment)
 *    ----------------------------------------------------------------------------------------------------------------------------------
 *    sem_open      op_open             Works on the proc counter semaphore [1] to wait and decrement the value by 1. (adjustment)
 *    ----------------------------------------------------------------------------------------------------------------------------------
 *    sem_close     op_close            Works on the lock semaphore [2] to wait for the value to become 0. (blocking)
 *                                      Also works on the lock semaphore [2] to increment the value by 1. (adjustment)
 *                                      Also works on the proc counter semaphore [1] to increment the value by 1. (adjustment)
 *
 *                  op_unlock           Works on the lock semaphore [2] to wait and decrement the value by 1. (adjustment)
 *    ----------------------------------------------------------------------------------------------------------------------------------
 *    sem_op        op_op               Works on the semaphore value (storing) semaphore [0] by incrementing it by 99. (adjustment)
 *                                      Somewhat untrue as the function definition modifies the 99 by the value parameter provided.
 *    ----------------------------------------------------------------------------------------------------------------------------------
 *
 * To illustrate the workflow of the interface used in main.c, let's breakdown the actual process steps:
 *    ->  sem_create is called first. The function is called with argument (SEMKEY, 1). 
 *        What this means is we want to create a semaphore set using the SEMKEY key, and the 
 *        semaphore value [0] to be 1. This is done when the first process calls sem_create. 
 *        Also, at the first call, the value of proc counter semaphore [1] is set to BIGCOUNT.
 *        NOTE: When other process calls the sem_create after it has been called previously, 
 *              the inital setup of [0] and [1] are not done. Rather, only the op_endcreate 
 *              operation is perfomed (which is performed in the first call as well.) This 
 *              will decrement the proc counter value by 1. So, in the first call to function,
 *              the value is set to (BIGCOUNT - 1), and in the second call, (BIGCOUNT - 2)...
 *        Returns the id of the semaphore maintained by the kernel.
 *    ->  sem_wait is called after entering the loop. This function calls the sem_op with arguments
 *        as (id, -1). id represents the semaphore id maintained by the kernel. -1 indicates the value 
 *        is first set as the sem_op member of the op_op struct (zero is not allowed), and the operation 
 *        is performed to first wait for the actual semaphore value [0] to be >= 1, and if that condition 
 *        is met, then decrement it by 1. (caller is put to sleep until condition is satisfied).
 *    ->  sem_signal is called at the end of for loop. After each iteration, the sem_signal is called which 
 *        calls the sem_op inside. The argument provided is (semid), which is the semaphore ID maintained by 
 *        the kernel. The sem_op function is called with argument (id, 1), where 1 signifies that the operation 
 *        is performed on the actual semaphore value [0], where the value is incremented by 1.
 *    ->  sem_close is called after the loop ends. The function is called with argument (semid). What this function 
 *        does is, op_close operation is performed. It first waits for the lock semaphore [2] to becomre zero (0)
 *        and increments the value later by 1. After this, the proc counter semaphore [1] is incremented by 1.
 *        The value of proc counter [1] sempahore is fetched. If the value is greater than BIGCOUNT, which it shouldn't,
 *        then the process is terminated. If it is equal to BIGCOUNT, then no other process is using the semaphore, so we 
 *        can call the sem_rm function, which removes the semaphore ID from the kernel (IPC_RMID). If the value is less 
 *        than BIGCOUNT, we know that other process is using the semaphore, so the op_unlock operation is performed, 
 *        which decrements the lock semaphore [2] by the value of 1. This signifies that the lock has been unlocked, and 
 *        no hinderance on other processes are done.
 *
 * Now, let's try to keep track of the semaphore values of the three members in the set:
 *
 *      Function        semaphore value [0]       process counter [1]       lock [2]      Adjustment          Description
 *      Initial               -                         -                     -               -                 Initial
 *    ->sem_create            0                         0                     0               -                 semget call
 *                            0                         0                     1             -1 [2]              op_lock
 *                        (initval) (1)           (BIGCOUNT) (10000)          1               -                 semval == 0
 *                            1                       9999                    0           +1 [1], +1 [2]        op_endcreate
 *      <case 1: another process calls sem_create>
 *    ->sem_create            1                       9999                    0                                 semget call
 *                            1                       9999                    1             -1 [2]              op_lock (requires [2] to be zero before it is incremented)
 *                            1                       9999                    1               -                 semval != 0
 *                            1                       9998                    0           +1 [1], +1 [2]        op_endcreate
 *      <end case>
 *    ->sem_wait              1                       9999                    0               -                 call sem_op(-1)
 *                            0                       9999                    0             +1 [0]              assign and set sembuf.sem_op and perform semop
 *    ->sem_signal            0                       9999                    0               -                 call sem_op(1)
 *                            1                       9999                    0             -1 [0]              assign and set sembuf.sem_op and perform semop
 *    ->sem_close             1                       10000                   1           -1 [1], -1 [2]        op_close (exited)
 *
 *      <case 1 after waiting in sem_wait, which requires the [0] to be >= 1, else put to sleep, observe that sem_close will work differently if there are processes in the queue. Also, notice from above that calls sem_wait, and sem_signal have no effect on [1].>
 *    ->pre sem_close         1                       9998                    0               -                 case: waiting for semaphore opertation in sem_wait.
 *    ->sem_close             1                       9999                    1           -1 [1], -1 [2]        op_close
 *                            1                       9999                    0             +1 [2]              op_unlock
        <first process which finished then terminates, and the other process continues...>
 *      <second process contiues from the sem_Wait and terminates when [1] == BIGCOUNT == 10000>
*/

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
  /*
   * Regarding op_lock:
   *    - First element has the member: {2, 0, 0}, which indicates the semaphore member to work on is the 
   *      third member [2]. The sem_op of 0 states that the process should wait until the semaphore value is zero (0),
   *      and if it is not zero, the process is put into sleep (as there is no IPC_NOWAIT flag specified).
   *    - Second element has the member: {2, 1, SEM_UNDO}, which indicates the semaphore member to operate on is the third member [2].
   *      The sem_op of 1 (positive) indicates that the process increments the semaphore's current by `sem_op` (1) value. SEM_UNDO
   *      indicates that the kernel keep an adjustment value for the semaphore for the edge cases described earlier.
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
  /*
   * Lets observe what sorts of operation is done. Notice that the op_endcreate has two elements.
   *    - For the first element, it contains the member: {1, -1, SEM_UNDO}, where the second semaphore [1] is waited till the semaphore
   *      number is >= 1. After that, the semaphore value is decremented by 1. The process is put to sleep until the condition mentioned 
   *      is satisified. The kernel will keep track of the adjustment value with the SEM_UNDO flag.
   *    - For the second element, it contains the member: {2, -1, SEM_UNDO}, where the third semaphore [2] is waited till the semaphore
   *      number is >= 1. After that, the semaphore value is decremented by 1. SEM_UNDO is explained above.
   *
   * NOTE That the semaphore value of the first member is not zero. The semaphore value is modified in the above if condition.
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
  /*
   * Let's try to understand the operation... Notice that op_open has one element.
   *    - The element contains the member: {1, -1, SEM_UNDO}, where the second member of the sempahore set [1] is the one where 
   *      the operation is carried out. The semaphore value is checked to see if it's >= 1. Unless that condition is satisfied, the
   *      caller is put into sleep. When the condition is satisfied, the semaphore value is decremented by 1. (absolute value of 
   *      the sem_op. (|-1| in this case)). SEM_UNDO is explained in earlier calls of `semop` calls.
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
  /*
   * The operation on semaphore is as follows (op_close contains three elements):
   *    - The first element: {2, 0, 0}, informs the operation on the third semaphore [2], and waits for the value of the 
   *      semaphore to be equal to zero. Unless the value is equal, the process is put into sleep.
   *    - The second element: {2, 1, SEM_UNDO}, informs the operation on the third semaphore [2], and increments the 
   *      semaphore value by 1 (the sem_op value provided). The SEM_UNDO is kernel adjustment flag.
   *    - The third element: {1, 1, SEM_UNDO}, informs the operation on the second sempahore [1], and increments the 
   *      value of the semaphore by 1 and kernel adjustment value is set too (SEM_UNDO).
  */
  if (semop(id, &op_close[0], 3) < 0) {
    err_sys("can't semop");
  }

  /*
   * Now that we have a lock, read the value of the process counter to see if this is the last reference to the semaphore.
   * There is a race condition here--see the comments in sem_create().
  */
  /*
   * semctl will fetch the value of the second member of the semaphore set (process counter) given by id.
  */
  if ( (semval = semctl(id, 1, GETVAL, 0)) < 0) {
    err_sys("can't GETVAL");
  }

  if (semval > BIGCOUNT) {
    err_dump("sem[1] > BIGCOUNT");
  } else if (semval == BIGCOUNT) {
    sem_rm(id);
  } else {
    /* 
     * when multiple processes call sem_open, the semaphore value of the second member (process counter) is decremented. 
     * 
     * What opeartion semop does is as follows (op_unlock has 1 element):
     *    - The element contains the following members: {2, -1, SEM_UNDO}, which operates on the third member (lock) and waits 
     *      till the semaphore value is >= 1. Until the condition is satisfied, the calling process is put into sleep, and when 
     *      the condition is satisfied, the value of the semaphore is decremented by absolute value of sem_op (|-1| in this case).
    */
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
  /*
   * Recall that the sem_op member of the `struct sembuf op_op` initially is 99 after initalization. 
  */
  if ( (op_op[0].sem_op = value) == 0) {
    err_sys("can't have value == 0");
  }

  /*
   * The semop operates on the op_op array which has one element:
   *    - The element contains the following member: {0, 99, SEM_UNDO}, BUT, if the value parameter is non-zero, then the 
   *      sem_op member is modified from 99 to the value parameter. This means that the first semaphore from the set [0] is the one
   *      where operation is done. If the (modified) sem_op is:
   *        - negative, the calling process waits until the semaphore value is >= absolute value of the sem_op parameter. When the condition is satisfied, then the semaphore value is decremented by the absolute value of sem_op. 
   *        - positive, the semaphore value is incremented by sem_op amount.
   *      SEM_UNDO informs the kernel to maintain the adjustment value.
  */
  if (semop(id, &op_op[0], 1) < 0) {
    err_sys("sem_op error");
  }
}
