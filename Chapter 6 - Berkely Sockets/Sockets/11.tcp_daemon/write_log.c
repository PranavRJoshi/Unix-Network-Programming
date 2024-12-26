#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int write_log (int logfd, const char *logmsg) {
  char  message[LOG_MSG_LEN];
  int   logmsg_len = strlen(logmsg);
  int   final_msg_len;

  /* Prepend the actual message by the boilerplate message */
  sprintf(message, "fd: %2d parent pid: %5d child pid: %5d message: ", logfd, getppid(), getpid());

  /* 
   * Concatenate the actual message to the prepended message string.
   * MSG_LEN represents the total size of message string. The function
   * `strlcat` implicitly checks for the length of `destination` and subtracts 
   * it from the `destination_size` which is again subtracted by 1.
   *
   *    ->  destination_size - strlen(destination) - 1.
   * 
   * Check out the manual for how other cases are handled.
  */
  strlcat(message, logmsg, LOG_MSG_LEN);
  final_msg_len = strlen(message);

  if (write(logfd, message, final_msg_len) != final_msg_len) {
    fprintf(stderr, "write error: failed to write to the log file");
    return -1;
  }

  return final_msg_len;
}
