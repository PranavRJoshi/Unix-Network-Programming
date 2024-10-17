#include "err_routine.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE     1024

/*
 * main:  Use this program to provide a file name to the input stream and the program
 *        will execute the cat command on the provided filename.
 *        Also, it will print out the current working directory.
*/
int main (void) {
  
  int   n;
  char  line[MAXLINE], command[MAXLINE + 10];
  FILE  *fp;

  /* First example */
  /*
   * Read the filename from standard input
  */
  if (fgets(line, MAXLINE, stdin) == NULL) {
    err_sys("filename read error");
  }

  /*
   * Use popen to create a pipe and execute the command
  */
  sprintf(command, "cat %s", line);
  /*
   * popen with the second argument (type) as "r" indicates the calling process reads the standard output of the command
   * If the type is "w", the calling process writes to the standard input of the command
  */
  if ( (fp = popen(command, "r")) == NULL) {
    err_sys("popen error");
  }

  /*
   * Read the data from the FILE pointer and write to standard output
  */
  while ((fgets(line, MAXLINE, fp)) != NULL) {
    n = strlen(line);
    if (write(1, line, n) != n) {
      err_sys("data write error");
    }
  }

  if (ferror(fp)) {
    err_sys("fgets error");
  }

  pclose(fp);


  /* Second example */
  /* chdir("/"); */   /* changes the current working directory to "/". Unmounts the filesystem */

  if ( (fp = popen("/bin/pwd", "r")) == NULL) {
    err_sys("popen error");
  }

  if (fgets(line, MAXLINE, fp) == NULL) {
    err_sys("fgets error");
  }

  printf("Current Directory: %s", line);   /* pwd inserts new-line */

  pclose(fp);

  return 0;
}
