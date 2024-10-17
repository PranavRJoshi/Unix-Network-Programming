#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void handle_sigint (int sig_id);

void handle_sigalrm (int sig_id);

volatile int sigint_flag = 0;

volatile int master_flag = 0;

int main (void) {

  signal(SIGALRM, handle_sigalrm);
  signal(SIGINT, handle_sigint);

  alarm(10);

  /* oldmask for reseting the signal. */
  int oldmask;

  /* For checking the pending signals  */
  int sig_count;
  sigset_t pending_signals;

  sigpending(&pending_signals);
  for (sig_count = 1; sig_count < NSIG; sig_count++) {
    if (sigismember(&pending_signals, sig_count)) {
      printf("[SIG PENDING] Signal %d is pending.\n", sig_count);
    }
  }

  /* 
   * If a SIGINT is received while sleep is executing, then the handler for SIGINT is invoked, and the sleep is terminated prematurely as well
   * 
   * As the man page states, the execution of the calling thread is suspended until arg seconds have passed or a signal is delivered to the thread
   * and its action is to invoke a signal-catching function or to terminate the thread or process.
  */
  sleep(10);

  /* Test if the signals are handled properly or not */
  for (;;) {
    oldmask = sigblock(sigmask(SIGINT) | sigmask(SIGALRM));
    while (sigint_flag == 0 && master_flag == 0) {
      /* For checking the pending signals */
      sigpending(&pending_signals);

      for (sig_count = 1; sig_count < NSIG; sig_count++) {
        if (sigismember(&pending_signals, sig_count)) {
          printf("\n[SIG PENDING2] Signal %d is pending.\n", sig_count);
        }
      }

      /* sleep for 2 seconds, make sure the loop isn't iteratred very fast */
      sleep(2);

      /* Pause the execution till the SIGALRM signal is received. The above sigblock that blocks the SIGALRM is overridden. So, the process can accept SIGALRM, but not SIGINT. */
      sigpause(SIGALRM);
    }
    printf("[LOG] The master flag may have been invoked...\n");
    break;
  }

  /* For checking the pending signals */
  sigpending(&pending_signals);

  for (sig_count = 1; sig_count < NSIG; sig_count++) {
    if (sigismember(&pending_signals, sig_count)) {
      printf("\n[SIG PENDING2] Signal %d is pending.\n", sig_count);
    }
  }


  printf("Resetting the signals, now, the SIGINT flag can be handled.\n");
  sigsetmask(oldmask);

  return 0;
}

void handle_sigint (int sig_id) {
  printf("\n[LOG] The value in sig_id from function handle_sigint is: %d\n", sig_id);
  master_flag = 1;
}

void handle_sigalrm (int sig_id) {
  printf("\n[LOG] Encountered the SIGALRM signal, handled by handle_sigalrm. sig_id: %d\n", sig_id);
  master_flag = 1;
}
