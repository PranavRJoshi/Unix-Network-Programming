#ifndef SHM_H
#define SHM_H

#include "mesg.h"

#define     NBUFF     4                 /* number of buffers in shared memory */
                                        /* (for multiple buffer version) */

#define     SHMKEY    ((key_t) 7890L)    /* base value of shm key */

#define     SEMKEY1   ((key_t) 7891L)    /* client semaphore key */
#define     SEMKEY2   ((key_t) 7892L)    /* server semaphore key */

#define     PERMS     0666              /* IPC access mode */

#endif
