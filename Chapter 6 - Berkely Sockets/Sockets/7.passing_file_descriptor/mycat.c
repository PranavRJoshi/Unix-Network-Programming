/*
 * Catenate one or more files to standard output.
 * If no files are specified, default to standard input.
*/

#include "err_routine.h"
#include "common.h"

#define BUFFSIZE    4096

int main (int argc, char **argv) {
  
  int         fd, i, n;
  char        buff[BUFFSIZE];
  extern char *pname;

  pname = argv[0];

  argv++;   /* point to next argument for program. If `./mycat /etc/passwd`, point to string `/etc/passwd` */
  argc--;   /* decrement argument count, with no additional args, argc still is 1 (program name). */

  fd  = 0;    /* default to stdin */
  i   = 0;

  do {
    if (argc > 0 && ((fd = my_open(argv[i], 0)) < 0)) {
      err_ret("can't open %s\n", argv[i]);
      continue;
    }
    
    while ( (n = read(fd, buff, BUFFSIZE)) > 0) {
      if (write(1, buff, n) != n) {
        err_sys("write error\n");
      }
    }

    if (n < 0) {
      err_sys("read error\n");
    }

  } while (++i < argc);   /* while argc is greater than i + 1, loop. Meaning, if other paths are specified, read from there too. */

  exit(EXIT_SUCCESS);
}
