#include "client.h"         /* for function: client */
#include "server.h"         /* for function: server */
#include "err_routine.h"    /* for function: err_sys */

#include <unistd.h>         /* for function: pipe, fork */
#include <sys/wait.h>       /* for function: wait */
#include <stdlib.h>         /* for function: exit */

/* 
 * main:  The process first creates two pipes: pipe1 and pipe2. It checks if the system call passed or failed.
 *        Next, the process forks itself, having two process. The parent process acts as a *client*, and the child as the *server*.
 *        When the fork is successful, the parent process (client) closes the reading pipe of pipe1 and writing pipe of pipe2. 
 *        The child process (server) closes the reading pipe of pipe2 and writing pipe of pipe1.
 *        The client has to wait for the server to respond, so waiting is performed.
 *        According to the manual, wait it defined as follows:
 *            If wait() returns due to a stopped or terminated child process, 
 *            the process ID of the child is returned to the calling process. 
 *            Otherwise, a value of -1 is returned and errno is set to indicate the error.
 *        The argument is NULL [(int *) 0] as we are not interested in the termination information of the child process.
 *
 * usage: After executing the main (./main), enter the filename that is located where the executable is located, and then the program returns the content of the file. NOTE: Only works with file which contains the ASCII characters (text file).
*/
int main (void) {
  int childpid, pipe1[2], pipe2[2];

  if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
    err_sys("can't create pipes");
  }

  if ( (childpid = fork()) < 0 ) {
    err_sys("can't fork");
  } else if (childpid > 0) {                  /* parent process */
    close(pipe1[0]);      /* close reading end for pipe1 */
    close(pipe2[1]);      /* close writing end for pipe2 */

    client(pipe2[0], pipe1[1]);

    while (wait((int *) 0) != childpid) {     /* wait for child */
      ;
    }

    close(pipe1[1]);      /* close writing end for pipe1 */
    close(pipe2[0]);      /* close reading end for pipe2 */
    exit(EXIT_SUCCESS);
  } else {                                    /* child process */
    close(pipe1[1]);      /* close writing end for pipe1 */
    close(pipe2[0]);      /* close reading end for pipe2 */

    server(pipe1[0], pipe2[1]);

    close(pipe1[0]);      /* close reading end for pipe1 */
    close(pipe2[1]);      /* close writing end for pipe2 */
    exit(EXIT_SUCCESS);
  }
}
