#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "err_routine.h"

int main (void) {
  int   pipefd[2], n;
  char  buff[100];

  /* pipefd[0] is used for reading, pipefd[1] is used for writing. */
  if (pipe(pipefd) < 0) {
    err_sys("pipe error");
  }

  /* 
   * printf buffers the content before sending it to the standard output. 
   * Apparently, the printf sends the buffer to standard output if:
   *    1. the buffer is full.
   *    2. a newline character (\n) is detected. (true for line-mode: terminals, not for block-mode: writing to disk.)
   *    3. there is impending (hanging over) input (on the same device).
   *
   * For furhter understanding, refer: https://stackoverflow.com/questions/45385807/what-is-it-with-printf-sending-output-to-buffer
  */
  printf("read fd = %d, write fd = %d", pipefd[0], pipefd[1]);

  if (write(pipefd[1], "hello world\n", 12) != 12) {
    err_sys("write error");
  }

  if ( (n = read(pipefd[0], buff, sizeof(buff))) <= 0 ) {
    err_sys("read error");
  }

  write(1, buff, n);      /* fd 1 = stdout */

  exit(EXIT_SUCCESS);
}
