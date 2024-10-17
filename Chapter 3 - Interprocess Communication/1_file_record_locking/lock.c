#include <unistd.h>         /* for lockf, getpid */
#include <sys/file.h>       /* for flock */
#include <sys/errno.h>      /* for errno */
#include <stdio.h>          /* for sprintf */
#include <fcntl.h>          /* for open and it's bitmasks */

extern int errno;

#include "lock.h"           /* for the function prototypes */
#include "err_routine.h"    /* for err_sys */

#define LOCKFILE  "seqno.lock"    /* name of the lock file, used by link (and creat) methods */
#define TEMPFILE  "temp.lock"     /* name of the temp lock file, used by the creat method */
#define PERMS     0666            /* file permission used for open system call */

void my_lock (fd)
int fd; {
  lseek(fd, 0L, 0);                           /* rewind before lockf */
  if (lockf(fd, F_LOCK, 0L) == -1) {          /* 0L -> lock entire file */
    err_sys("can't F_LOCK");
  }
}

void my_unlock (fd)
int fd; {
  lseek(fd, 0L, 0);
  if (lockf(fd, F_ULOCK, 0L) == -1) {
    err_sys("can't F_ULOCK");
  }
}

void my_sys_call_lock (fd) 
int fd; {
  if (flock(fd, LOCK_EX) == -1) {
    err_sys("can't LOCK_EX");
  }

  /* 
   * NOTE:  If you use LOCK_SH instead of LOCK_EX, and run two instances of the program,
   *        then the processes will run in a non-sequential manner, like if PID of 100 and 101 
   *        are the process IDs of the instances of the program, you'll see the output similar 
   *        to the one when no lock was used.
   *        
   *        But if LOCK_EX was used, the instances of the program will run in a sequential manner.
   *        PID of 100 (suppose) will run and complete till it reaches the unlock function, and 
   *        afterwards PID of 101 (suppose) will start the execution.
  */
}

void my_sys_call_unlock (fd) 
int fd; {
  if (flock(fd, LOCK_UN) == -1) {
    err_sys("can't LOCK_UN");
  }
}

void my_link_lock (fd) 
int fd; {
  int   tempfd;
  char  tempfile[30];

  sprintf(tempfile, "LCK%d", getpid());     /* Stores the "LCK<pid>" in the char array tempfile */

  /*
   * Create a temporary file, then close it.
   * If the temporary file already exists, the creat() will 
   * just truncate it to 0-length.
  */
  if ( (tempfd = creat(tempfile, 0444)) < 0) {
    err_sys("can't creat temp file");
  }
  close(tempfd);

  /*
   * Now, try to rename the temporary file to the lock file.
   * This will fail if the lock file already exists (i.e., if some other process already has a lock)
  */
  while (link(tempfile, LOCKFILE) < 0) {
    /* The link system call fails if the name of the new link to the file already exists. */
    if (errno != EEXIST) {
      err_sys("link error");
    }
    sleep(1);
  }

  /* Here's what's essentially happening when we're "locking" the file.
   * 1. First thing, a character array tempfile is filled with "LCK<pid>" as the string.
   * 2. Then, the creat system call will create a file that has the name as that of the string stored in tempfile variable.
   *    It should also be noted that we created file with file creation mask of 444, and the typical umask is 022. So, the file 
   *    created should have the file creation mask of 422.
   *    One more thing to note is that, although the file created will have different name thanks to using PID in the name, if the name did already exist, creat will truncate the contents of the file and essentially act like re-creating the file.
   * 3. Next, the link system call is invoked, which--according to the manual--creates the specified directory entry (hard link) the second argument with the attributes of the underlying object pointed at by the first argument. Upon successful call, the link system call returns 0, but it returns -1 if an error occured and sets the errno.
   *    The reason we check for error other than EEXIST is because EEXIST indicates the linked named by second argument already exists. It means, if one process has already linked with seqno.lock (in our example) with the first argument to link, then some other process is trying to link with seqno.lock with their respective first argument, the system call will fail and the errno will be set ot EEXIST. The program deals with this by sleeping for 1 second.
   * 4. Finally, the unlink system call is invoked. From the manual, this system call removes the link named by path from it's directory and decrements the link count of the file which was referenced by the link. (I didn't mention this, but the link count of the first argument of the `link` system call, when successful, is incremented.). Also, from the manual, if the decrement reduces the link count of the file to zero, and no process has the file open, then all resources associated with the file are reclaimed. (probably by the OS)
   *    In this case, the tempfile, which holds the file containing the PID is unlinked, but as the manual (of link) states that, if argument 1 is removed, the file (argument 2) is not deleted and the link count of the underlying object is decremented.
  */

  if (unlink(tempfile) < 0) {
    err_sys("unlink error for tempfile");
  }
}

void my_link_unlock (fd) 
int fd; {
  /*
   * Removes the "seqno.lock" file
  */
  if (unlink(LOCKFILE) < 0) {
    err_sys("unlink error for LOCKFILE");
  }
}

void my_creat_lock (fd)
int fd; {
  int tempfd;

  /*
   * Try to create a temporary file, with all write permissions turned off.
   * If the temporary file already exists, the creat() will fail.
  */
  while ( (tempfd = creat(TEMPFILE, 0)) < 0 ) {
    /*
     * So the way this lock and unlock works is with the way creat works. Suppose that two processes are working on the same file.
     * This technique neither uses the concept of link, lockf or flock. What it does is create a dummy file called "temp.lock"
     * The mode of the file is given 0, meaing the file does not have any read or write access (super-user has access still). 
     * Whichever of the process creates the file first, it has the "lock" as the other process, when trying to create the file
     * gets an error EACCES as the file already exists and the file does not have read/write permission.
     *
     * NOTE: The caller does not need read/write permission for the file being unlinked.
     *
     * If the errno is set to anything but EACCES, the program is terminated.
    */
    if (errno != EACCES) {
      err_sys("creat error");
    }
    sleep(1);
  }
  close(tempfd);
}

void my_creat_unlock (fd)
int fd; {
  if (unlink(TEMPFILE) < 0) {
    err_sys("unlink error for tempfile");
  }
}

void my_open_lock (fd)
int fd; {
  int tempfd;

  /*
   * Try to create the lock file, using open() with both O_CREAT (create file if it doesn't exist) 
   * and O_EXCL (error if create and file already exists).
   *
   * If this fails, some other process has the lock.
  */
  while ( (tempfd = open(LOCKFILE, O_RDWR | O_CREAT | O_EXCL, PERMS)) < 0) {
    if (errno != EEXIST) {
      err_sys("open error for lock file");
    }
    sleep(1);
  }
  close(tempfd);
}

void my_open_unlock (fd)
int fd; {
  if (unlink(LOCKFILE) < 0) {
    err_sys("unlink error for lock file");
  }
}

