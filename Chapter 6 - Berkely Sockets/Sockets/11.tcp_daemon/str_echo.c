#include "utils.h"
#include <stdio.h>
#include <string.h>

#define MAXLINE   512
#define QUIT_CMD  "tcp quit"

void str_echo (int sockfd, int logfd) {
  int   n;
  char  line[MAXLINE];

  for (;;) {
    n = readline(sockfd, line, MAXLINE);

    if (n == 0) {
      write_log(logfd, "Client did not send any data.\n");
      return;
    } else if (n < 0) {
      write_log(logfd, "read error.\n");
      return;
    }

    /*
     * Added a feature to check if the client sent the "quit command".
     * If the command is sent to the server, the server will close the connection with the client.
     * And yeah, `readline` stores the C-string into `line` argument.
    */
    if (strncmp(line, QUIT_CMD, strlen(QUIT_CMD)) == 0) {
      write_log(logfd, "Client sent the \"QUIT COMMAND\".\n");
      return;
    }

    if (writen(sockfd, line, n) != n) {
      write_log(logfd, "Server couldn't write all the data to the client.\n");
      return;
    }
  }
}
