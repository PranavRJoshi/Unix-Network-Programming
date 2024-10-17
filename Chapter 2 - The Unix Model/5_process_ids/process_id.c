/*
 * Usage: 
 *  ->  To prepare the executable, run the `make` command.
 *  ->  Execute the `./process_id` program.
 *  ->  To remove the executable, run `make clean` command.
*/

#include <stdio.h>
/* getpid(), getppid() is declared in the unix standard header <unistd.h> */

int getpid();
int getppid();

main () {
  /* typrical process id for a process ranges from 0 to 30,000 */
  /* process id of 1 is a special process called the `init` process */
  /* process id of 0 is a special kernel process termed either the "swapper" or the "scheduler" */
  /* On virtual memory implementations of Unix, the process of process id 2 is kernel process called "pagedaemon" */
  /* Other process id does not hold any significant meaning */
  printf("The current process id is: %d\n", getpid());
  printf("The parent process id is: %d\n", getppid());    /* returns the process id of the shell running the program, for me, it was `/bin/zsh`. use `ps` to check */
}
