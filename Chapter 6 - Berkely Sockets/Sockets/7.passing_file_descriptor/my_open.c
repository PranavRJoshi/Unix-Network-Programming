#include "err_routine.h"
#include "common.h"

extern int errno;

int my_open(char *filename, int mode) {
  int           fd, childpid, sfd[2], status;
  char          argsfd[10], argmode[10];
  extern int    errno;

  if (s_pipe(sfd) < 0) {      /* create an unnamed stream pipe */
    return (-1);              /* errno will be set */
  }

  if ((childpid = fork()) < 0) {
    err_sys("can't fork");
  } else if (childpid == 0) {

    /*
     * Notice that the child process's Process ID (pid) is identical to the return value of 
     * wait() system call. Also realize that the `childpid` variable will be `0` only for the child 
     * process, for the parent process, the `childpid` variable will hold the process ID of the child 
     * process.
    */
    // fprintf(stderr, "log: the child process's pid is: %d\n", getpid());

    close(sfd[0]);
    snprintf(argsfd, sizeof(argsfd), "%d", sfd[1]);
    snprintf(argmode, sizeof(argmode), "%d", mode);
    
    if (execl("./openfile", "./openfile", argsfd, filename, argmode, (char *) 0) < 0) {
      err_sys("can't execl");
    }
  }

  /* parent process - wait for the child's execl() to complete */

  close(sfd[1]);        /* close the end we don't use */

  // fprintf(stderr, "log: the value of childpid is: %d\n", childpid);

  /*
   * The `status` variable will contain the exit information of the terminated child process.
   * For now, we won't have to worry about termiantion due to signals.
  */
  if (wait(&status) != childpid) {
    err_dump("wait error");
  }

  // fprintf(stderr, "log: the status returned from child process is: %d\n", status);

  /*
   * In the `openfile.c` file, you can see that if the `open` system call fails, and if 
   * errno <= 0, then the `openfile` executable will return 255. 
   *
   * If status is 255 (in case `open` failed with no errno set), then the below operation
   * `status & 255` will result in `1`.
   *
   * One more thing to add, the call to `my_sendfile` may also result in receiving 255, as 
   * the `sendmsg` call, in case it was to fail with errno <= 0, returns 255.
   *
   * If any of the first 8 bits (bit 0 to bit 7) is set, then some other form of error has occured.
   * This is becuase the `status` variable encodes the child process's termination information.
   * For instance, if the `WIFSIGNALED(x)`, where `x` is the `status` variable, first bitwise 
   * AND with `0177` (decimal equivalent to 127 or the first 7 bits set), and checks if it is 
   * equal to `_WSTOPPED`. If it is not equal, logical AND is done to check if `_WSTATUS(x)` is 
   * not equal to zero. Kinda messy explanation, so let's look at an example. We'll first consider 
   * that the `status` variable has the first 7 bits set, we won't care about remaining bits and 
   * thus represent it with X (don't care):
   *
   *    `WIFSIGNALED` is defined as:
   *        #define WIFSIGNALED(x)  (_WSTATUS(x) != _WSTOPPED && _WSTATUS(x) != 0)
   *
   *    status = 0b-XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-X111-1111
   *
   *    _WSTATUS(x) is defined as:
   *        #define _WSTATUS(x)     (_W_INT(w) & 0177)
   *
   *    This seems more complicated that it should be. `_W_INT(w)` macro is defined as:
   *        #define _W_INT(w)       (*(int *)&(w))
   *    
   *    All this does is, cast the address of `w` into an integer pointer, and then dereference 
   *    to get the object pointed at that address.
   *    
   *    Getting back to the point, `0177` is an octal representation, whose binary representation 
   *    is 0b-0000-0000-0000-0000-0000-0000-0111-1111.
   *
   *    Observe that `_WSTATUS(status)` will be `0177`. Also, _WSTOPPED is defined as: 
   *        #define _WSTOPPED       0177
   *
   *    This means that `_WSTATUS(status) != _WSTOPPED` evaluates to `0` as both are identical. 
   *    Due to short-circuit nature of logical operation, the next expression is not evaluated 
   *    since `0 && <any expression>` will result in `0`.
   *
   *    This means that the `WIFSIGNALED(x)` macro will return 0, signifying that the child process 
   *    did not receive any signal.
   *
   *    A small note: Seems like when a child process is "interrupted" by a signal, bits 8-15 are 
   *    filled with signal number (see <sys/signal.h>). I won't go into much detail for now.
   *
   *    One could also observe that, in case that not all bits, from bit 0 to bit 7 are set, then 
   *    _WSTATUS(x) will return a non-zero value. This means any value < 0177 will result in the 
   *    _WSTATUS(x) returning the corresponding number, which also won't be equal to _WSTOPPED (0177).
   *    In this case, the second expression is also evaluated: `_WSTATUS(x) != 0` for WIFSIGNALED(x).
   *    This will return a non-zero value as before. 
   *
   * For further reading and decoding, check out the <sys/wait.h> header file to see various parameterized macro 
   * used for handling the exit status of the child.
   *
   * The main point of this entire discussion is that, if any of the lower order bits (bit 0 to bit 7) are set, 
   * then some other error has error.
   *
   * If not, we know that the process terminated with some normal `exit` call, and bit 8 to bit 15 contains the 
   * exit status (or return value) from the child process. Hence, we right shift the bits 8 times such that 
   * bit 8 to bit 15 are now in position of bit 0 to bit 7 respectively (status >> 8). This is later bitwise 
   * AND with 255 (all bits from bit 0 to bit 7 set) to check the exit code. On normal termination, the 
   * child process returns 0, so, this operation will return 0, but for any other scenario, the corresponding 
   * status code is stored (status = (status >> 8) & 255). This is why we check if status is 0, and only then 
   * we call the `my_recvfile()` function from the parent process, else we return the child process's returned 
   * exit status is stored in errno and -1 is returned to caller.
  */
  if ((status & 255) != 0) {
    err_dump("child did not exist");
  }

  status = (status >> 8) & 255;     /* child's exit() argument */

  if (status == 0) {
    fd = my_recvfile(sfd[0]);          /* all OK, receive fd */
  } else {
    errno = status;     /* errno, ser errno value from child's errno */
    fd = -1;
  }

  close(sfd[0]);        /* close the stream pipe */
  return (fd);

}
