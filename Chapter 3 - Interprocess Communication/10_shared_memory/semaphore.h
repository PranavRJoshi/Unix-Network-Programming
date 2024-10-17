/*
 *  Provide a simpler and easier to understand interface to the System V semaphore system calls. 
 *  There are 7 routines available to the user:
 *    1.  id = sem_create(key, initval);    // create with initial value or open
 *    2.  id = sem_open(key);               // open (must already exists)
 *    3.  sem_wait(id);                     // wait = P = down by 1. (Djikstra's `(P)robeer` (try) operation)
 *    4.  sem_signal(id);                   // signal = V = up by 1. (Djikstra's `(V)erhoog` (increment) operation)
 *    5.  sem_op(id, amount);               // wait if (amount < 0)
 *                                          // signal if (amount > 0)
 *    6.  sem_close(id);                    // close
 *    7.  sem_rm(id);                       // remove (delete)
*/

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

/* includes */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include "err_routine.h"

extern int errno;

/* Function declarations */

/*
 * sem_create:  Create a semaphore with a specified initial value.
 *              If the semaphore already exists, we don't initialize it (of course).
 *              We return the semaphore ID if all OK, else -1.
*/
int sem_create (key_t key, int initval);

/*
 * sem_open:  Open a semaphore that must already exist.
 *            This function should be used, instead of sem_create(), if the caller knows that the semaphore must
 *            already exist.
 *            For example, a client from a client-server pair would use this, if its the server's responsibility
 *            to create the semaphore.
 *            We return the semaphore ID if all OK, else -1.
*/
int sem_open (key_t key);

/*
 * sem_rm:  Remove a semaphore. 
 *          This call is intended to be called by a server, for example, when it is being shut down, as we do an
 *          IPC_RMID on the semaphore, regardless whether other processes may be using it or not.
 *          Most other processes should use sem_close() below.
*/
void sem_rm (int id);

/*
 * sem_close: Close a semaphore.
 *            Unlike the remove function above, this function is for a process to call before it exits, when it 
 *            is done with the semaphore.
 *            We "decrement" the counter of processes using the semaphore, and if this was the last one, we can
 *            remove the semaphore.
*/
void sem_close (int id);

/*
 * sem_wait:  Wait until a semaphore's value is greater than 0, then decrement it by 1 and return.
 *            Djikstra's P operation. Tanebaum's DOWN operation.
*/
void sem_wait (int id);

/*
 * sem_signal:  Increment a semaphore by 1.
 *              Djikstra's V operation. Tanebaum's UP operation.
*/
void sem_signal (int id);

/*
 * sem_op:  General semaphore operation.
 *          Increment or decrement by a user-specified amount (positive or negative; amount can't be zero).
*/
void sem_op (int id, int value);

#endif
