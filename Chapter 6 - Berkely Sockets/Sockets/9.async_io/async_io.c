/*
 * Copy standard input to standard output, using asynchronous I/O.
*/

/*
 * Some miscellaneous note about signals in my machine.
 *
 * 1. The `sigset_t` is a type definition for 32-bit unsigned integer.
 * 2. The `sigemptyset(ptr to sigset_t)` initializes the object referenced by `ptr` to `0`. 
 * 3. The `sigaddset(ptr, signum)` macro is a bit tricky as it uses the bitwise inclusive OR operation. But, let's try to understand it.
 *    - The `ptr` is dereferenced. The object referenced by `ptr` is bitwise inclusive OR'd with `__sigbit(signum)`. Note that 
 *      bitwise inclusive OR operation returns 0 only if both bits are 0, unlike exclusive OR (^), which returns 0 if both bits are 
 *      1 or both bits are 0.
 *    - Moving on to `__sigbit(signum)` function, which returns 0 if `signum` is > 32 (__DARWIN_NSIG), else returns:
 *        
 *        1 << (`signum` - 1)
 *
 *      This means that 1, i.e.     0b-0000-0000-0000-0000-0000-0000-0000-0001 
 *      is shifted by, say `SIGIO's signum` - 1, which is `23 - 1 = 22`. 
 *      So, the overall result is:  0b-0000-0000-0100-0000-0000-0000-0000-0000
 *      The hex represenation is:   0x-00-40-00-00.
 *
 *    - The result obtained above is bitwise inclusive OR'd with the sigset_t object (`*ptr`) that was passed to `sigaddset()` macro.
 * 4. The `sigismember(ptr, signum)` is used to check if the given `signum` is available in the object referenced by `ptr`. 
 *    Internally, it uses the bitwise AND operator to check if the bit in the object corresponding to `__sigbit(signum)` is set.
 *    If it is set, then the macro returns non-zero (must be 1 as it's the only other bit) value, else returns 0 if the bit
 *    in the object referenced by `ptr` is not set.
 * 5. The system calls used are `sigprocmask` and `sigsuspend`. These are the POSIX compliant call, unlike the ones used in 
 *    the text. The calls `sigblock`, `sigpause`, and `sigsetmask` are all considered "deprecated" based on the respective 
 *    manual page.
*/

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define BUFFSIZE    4096

int sigflag;
void sigio_func (int signum);

int main (void) {

  int   n;
  char  buff[BUFFSIZE];
  time_t before_signal, after_signal;
  /*
   * Minor change from the text.
   * So, the text declares the function `sigio_func` inside the main function as:
   *
   *    int sigio_func();
   * 
   * This function is not *technically* valid for the signal handler function as the signal handler is 
   * expected to have a function which takes an integer as argument as well as returns nothing (void). 
   * Although the above function declare does not produce any error, it does throw a warning. 
   * 
   * The change I did was declare the function outside the main function, and have a function pointer `sigio_func_ptr`
   * hold the address of the function `sigio_func`. Realize that the `signal` function expects a pointer to the signal 
   * handler (the second argument), so we can pass `sigio_func_ptr` directly.
   */
  void  (*sigio_func_ptr) (int);
  sigio_func_ptr = sigio_func;

  signal(SIGIO, sigio_func_ptr);      /* Install signal handler for SIGIO */

  if ( fcntl(0, F_SETOWN, getpid()) < 0) {      /* 0 -> standard input stream */
    perror("F_SETOWN error");
    return (-1);
  }

  int stdin_flags;

  /*
   * Some more changes from the text...
   * So, the text directly sets the `FASYNC` flag using `F_SETFL` command. I also mentioned in the 
   * comment below that one should probably use the `F_GETFL` to fetch the previous flag and then 
   * bitwise OR with the `FASYNC` to retain all the flags. Now, the `fcntl` call returns the flags 
   * of the given file descriptor if the command is `F_GETFL`. Here, it is stored in `stdin_flags`
   * variable.
   *
   * Next, I checked if the flag `FASYNC` is already set for the file descriptor, which can be seen 
   * in the code below where `stdin_flags` is bitwise AND with FASYNC. The expression returns `1` if 
   * the flag is set, else returns `0`. 
   *
   * If the standard input did not have the `FASYNC` flag set, then it would be set.
  */
  if ((stdin_flags = fcntl(0, F_GETFL)) < 0) {
    perror("F_GETFL error");
    return (-1);
  }

  if (stdin_flags & FASYNC) {
    fprintf(stderr, "log: standard input already has the FASYNC flag set.\n");
  } else {
    fprintf(stderr, "log: standard input did not have FASYNC flag set. Setting...\n");
    if ( fcntl(0, F_SETFL, stdin_flags | FASYNC) < 0) {         /* could also use F_GETFL to fetch the previous flag and `|` with FASYNC */
      perror("F_SETFL error");
      return (-1);
    }
  }


  /* Signal set which contains the SIGIO the process will `SIG_BLOCK` inside the infinite for loop */
  sigset_t    mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGIO);

  /* Signal set which contains no signals which the process will use to `suspend`  */
  sigset_t    suspend_mask;
  sigemptyset(&suspend_mask);

  /* Signal set which initially contains no signal, but later holds the signal set which was before setting `mask` */
  sigset_t    prev_set;
  sigemptyset(&prev_set);

  /* Signal set used for holding the "previous" set before the for loop reaches the end and `prev_set` is SIG_SETMASK */
  sigset_t    check_set;
  sigemptyset(&check_set);

  for (;;) {
    /* sigblock(sigmask(SIGIO)); */     /* block the delivery for SIGIO */

    /* NOTE: POSIX Compliant alternative below. */

    /*
     * int sigprocmask (int how, const sigset_t *restrict set, sigset_t *restrict oset); 
     *
     * Since `set` is not null, action of sigprocmask() depends on `how`. As `how` is SIG_BLOCK, the new mask is the 
     * union of the current mask and the specified set (which has the SIGIO signal added). 
     *
     * Added the `oset` just to check what the old signals were before the union took place.
    */
    sigprocmask(SIG_BLOCK, &mask, &prev_set);

    /*
     * Macro declaration:     #define sigismember(set, signo) ...
     *
     * The way `sigismember` macro works is by fetching the object (sigset_t) referenced by `set` parameter.
     * The object is then bitwise AND with `__sigbits(signo)`, and if the operation is not zero, i.e., if the 
     * bit is already set, then the return value is also non-zero (must be 1), else the return value is 0.
    */
    if (sigismember(&prev_set, SIGIO)) {
      fprintf(stderr, "log: SIGIO was previously added to the set and value of prev_set is 0x%X.\n", prev_set);
    } else {
      fprintf(stderr, "log: SIGIO was not previously added to the set and value of prev_set is 0x%X.\n", prev_set);
    }

    while (sigflag == 0) {
      
      before_signal = time((time_t *) 0);

      // sigpause(0);                /* wait for a signal */ /* don't block any signals */
      
      /* POSIX compliant alternative below */

      /*
       * System call declaration:  int sigsuspend (const sigset_t *sigmask);
       *
       * This system call temporarily changes the blocked signal mask to the set to which sigmask points, and then waits for a 
       * signal to arrive; on return the previous set of masked signals is restored.
       *
       * What this means is that, as we are supplying object referenced by `suspend_mask`, which is empty, we are essentially telling 
       * the system to not block any signals and just wait for any signal to arrive.
       *
       * One could try to supply the object referenced by `mask` variable--which has the SIGIO signal set--to verify that the call
       * will indeed block the signal SIGIO from arriving.
      */
      sigsuspend(&suspend_mask);
    }

    /* 
     * We're here if (sigflag != 0). Also, we know that the SIGIO signal is currently blocked.
    */
    after_signal = time((time_t *) 0);
    if ( (n = read(0, buff, BUFFSIZE)) > 0) {
      if (write(1, buff, n) != n) {
        perror("write error");
        return (-1);
      }
    } else if (n < 0) {
      perror("read error");
      return (-1);
    } else if (n == 0) {
      return (0);           /* EOF */
    }

    sigflag = 0;

    // sigsetmask(0);        /* and reenable signals */   /* as it is 0, no signals are `set` to be blocked from delivery */
    sigprocmask(SIG_SETMASK, &prev_set, &check_set);

    if (sigismember(&check_set, SIGIO)) {
      fprintf(stderr, "log: SIGIO mask was set. Now it's reset and value of check_set is 0x%X.\n", check_set);
    } else {
      fprintf(stderr, "log: SIGIO mask was not set. Every signal is reset anyways and value of check_set is 0x%X.\n", check_set);
    }
    fprintf(stderr, "log: difference in time between waiting and receiving the signal is: %.0lf\n", difftime(after_signal, before_signal));
  }

  return (0);
}

void sigio_func (int signum) {      /* changed from `int sigio_func` to adapt to second argument for `signal (2)` */
  sigflag = 1;          /* just set flag and return */

  // fprintf(stderr, "log: got the SIGIO signal\n");

  /*
   * the 4.3BSD signal facilities leave this handler enabled for any further SIGIO signal.
  */
}
