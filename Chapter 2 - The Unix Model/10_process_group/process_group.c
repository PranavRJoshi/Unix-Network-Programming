/*
 * Usage: 
 *  ->  Prepare the executable using the `make` command in the current working directory.
 *  ->  execute `./process_group`
 *  ->  To clean, run `make clean`
*/
#include <stdio.h>
#include <unistd.h>

/* pid_t is a typedef of int */

int main (void) {

  pid_t process_group;
  pid_t process_group_id;
  process_group = getpgrp();
  process_group_id = getpgid(getpid());
  printf("getpgrp return value is: %d\n", process_group);
  printf("getpgid return value is: %d\n", process_group_id);

  /*
   * Testing purpose. 
   * A job-control shell that is a login shell makes each child process the leader of its own process group. 
   * Upon uncommenting the code below and executing the program, the process id field and process group id field will be identical.
  */
  /* printf("Process ID = %d, Process Group ID = %d, Parent Processs ID = %d\n", getpid(), getpgrp(), getppid()); */

  return 0;
}
