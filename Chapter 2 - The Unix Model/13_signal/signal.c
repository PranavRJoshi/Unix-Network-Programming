#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void my_intr (int);

void update_flag (int sig_id);

void update_hangup_flag (int sig_id);

volatile int sigint_flag = 0;
volatile int sighup_flag = 0;

int main (void) {
  
  /* Set the initial alarm handler (my_intr) when SIGALRM is "catched" */
  /* This is required to ensure that the correct signal handler for SIGALRM (in this example) is used, before the signal is "caught" */
  signal(SIGALRM, my_intr);

  signal(SIGINT, update_flag);
  
  signal(SIGHUP, update_hangup_flag);

  int oldmask;

  sigset_t cur_signals;

  /* Blocking the signals interrupt, quit, segmentation violation, and alarm. */
  /* oldmask = sigblock(sigmask(SIGINT) | sigmask(SIGQUIT) | sigmask(SIGSEGV) | sigmask(SIGALRM)); */ 
  /* Blocking the signal quit and alarm. That is why the handler for the SIGALRM is invoked at the end. */
  oldmask = sigblock( sigmask(SIGQUIT) | sigmask(SIGALRM) );
  sigprocmask(0, NULL, &cur_signals);   /* Receives the current signal mask as the second arg is NULL and first arg is insignificant */
  printf("[LOG] The previous signal bit mask is: %d\n", oldmask);
  printf("[LOG] The function sigprocmask has returned the value: %d\n", cur_signals);

  alarm(2);     /* sets a time to deliver the signal SIGALRM to the calling process after 2 seconds (in this example) */
  
  printf("The signal has not been handled.\n");

  /* while (1) ; */   /* Infinite loop */

  sleep(3);

  printf("The signal should have been handled by now...\n");

  for (;;) {
    sigblock(sigmask(SIGINT));
    sigprocmask(0, NULL, &cur_signals);
    printf("[LOG] The mask inside the for loop is: %d\n", cur_signals);
    /* printf("[LOG] The previous signal bitmask is: %d\n", oldmask); */
    /* Infinite loop unless SIGINT (CTRL + C) is received. */
    while (sigint_flag == 0) {
      if (sighup_flag != 0) {
        printf("Encountered the SIGHUP flag.\n");
        break;
      }
      /* printf("Waiting for SIGINT...\n"); */
      write(1, "Hello, World!\n", 14);
      sleep(2);
      /* 
       * From my understanding, the sigpause function takes in the argument which removes
       * the respective signal mask from the masks. 
       *
       * In this example, before the while loop is initiated, the sigblock function masks the SIGINT, 
       * meaning that any SIGINT encountered will be "blocked". After the sigpause function has been 
       * invoked, the mask for the SIGINT is removed, hence, the handler for the SIGINT will be initiated
       * once the SIGINT signal arrives to the process.
       *
       * To make this program again block the SIGINT, call setpause as: setpause(0). Now, try to provide the 
       * SIGINT using the CTRL + C, it should not respond to the signal and the program will not call the 
       * handler for the SIGINT.
      */
      /*
       * What amuses me is, when sigpause(0) is used, the while loop iterates multiple times, printing the first
       * statement inside the while loop is prinited after every interval. But, when using the sigpause(sigmask(SIGINT)) 
       * statement, the while loop is only iterated once, printing the message only once. The initial assumption was that 
       * as printf is unsafe, it may have been the reason, but using the write syscall behave similarly as well. 
      */
      sigpause(SIGINT);
      /* sigpause(0); */
    }
    /* signal handler for SIGINT or SIGHUP has been invoked. */
    printf("\nThe flag has been modified!\n");
    break;
  }

  /* if (signal(SIGINT, SIG_IGN) != SIG_IGN) signal(SIGINT, my_intr); */

  /* Restore the previous signal mask */
  sigsetmask(oldmask);

  return 0;
}

void my_intr (int a) {
  signal(SIGALRM, SIG_IGN);                                           /* ignore this signal */
  fprintf(stderr, "Encountered the SIGALRM signal interrupt.\n");
  signal(SIGALRM, my_intr);                                           /* reinstall the signal */
}

void update_flag (int sig_id) {
  /* write(1, "kill me SIGKILL\n", 16); */
  sigint_flag = 1;
}

void update_hangup_flag (int sig_id) {
  sighup_flag = 1;
}
