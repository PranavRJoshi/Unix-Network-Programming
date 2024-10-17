/*
 * Usage: 
 *  ->  Prepare the executable using the `make` command in the current working directory.
 *  ->  execute `./fork`
 *  ->  To clean, run `make clean`
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int main (void) {

  /* variable declaration and initialization */
  int pid;

  pid = fork();

  if (pid == -1) {
    printf("Fork Failed.\n");
    return 0;
  }

  if (pid == 0) {   /* child process */
    printf("[CHILD] This is the child process.\n");
    printf("[CHILD] The process id is: %d and the parent process id is: %d\n", getpid(), getppid());
    exit(EXIT_SUCCESS);
  } else {          /* parent process */
    printf("[PARENT] This is the parent process.\n");
    printf("[PARENT] The process id is: %d and the child process id is: %d\n", getpid(), pid);
    exit(EXIT_SUCCESS);
  }
}

