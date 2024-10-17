#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main (void) {

  int pid;

  if ((pid = fork()) == -1) {
    printf("[ERROR] Fork Failed.");
    return 0;
  }

  if (pid == 0) {
    printf("[CHILD LOG] Launching the dummy executable...\n");
    if (execve("./dummy", NULL, NULL) == -1) {
      printf("[ERROR] Failed to execute a program in the child process.");
      return 0;
    }
    /* Will not be executed as the process executed another program file above. */
    printf("[CHILD LOG] Finished Process.\n");

    /* Code fragment to check the behavior of wait and orphaned process */
    /* If wait is not provided in the parent process, the code below will provide 1, else parent process ID waiting for the child process. */
    /* 
    sleep(10);
    printf("The parent process id now is: %d\n", getppid());
    sleep(5);
    exit(EXIT_FAILURE);
    */
  } else {
    printf("[PARENT LOG] This is the parent process.\n");
    printf("[PARENT LOG] Executing the ls command...\n");

    /* weird quirk, but the second argument to execlp expects the program name--ls in this case--instead of argument to ls. */
    if (execlp("ls", "ls", "-la", (char *) 0) == -1) {
      printf("[ERROR] Failed to execute a program in the parent process.");
      return 0;
    }
    /* Will not be executed as the process executed another program file above. */
    printf("[PARENT LOG] Finished Process.\n");

    /* Code fragment to check the behavior of wait and orphaned process. */
    /*
    int wait_flag;
    int wait_status;
    */
    /* wait_flag = wait(&wait_status); */   /* wait_flag contains the terminated child process's ID and wait_status is dependent on the exit function's argument provided in the child process. */
    /* printf("[PARENT LOG] The wait_status is: %d and wait_flag is: %d\n", wait_status, wait_flag); */
    /* exit(EXIT_SUCCESS); */
  }

  return 0;
}
