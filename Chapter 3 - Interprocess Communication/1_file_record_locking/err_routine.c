#include <stdio.h>
/* #include <varargs.h> */  /* Use stdarg instead. */
#include <stdarg.h>
#include <stdlib.h>

#include "systype.h"

char *pname = NULL;

/*
 * Fatal error. Print a message and terminate
 * Don't dump core and don't print the system's errno value.
 *
 *        err_quit(str, arg1, arg2, ...) 
 *
 * The string "str" must specify the conversion specification for any args
*/

/* VARARGS1 */ 
/* NOTE: The va_dcl parameter specified is no longer supported as GCC has stopped the support for varargs.h */
/* Refer to this site: https://pubs.opengroup.org/onlinepubs/7908799/xsh/varargs.h.html */
/*
err_sys (va_alist)
va_dcl
{
  
}
*/

void err_sys (char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (pname != NULL) {
    fprintf(stderr, "%s: ", pname);
  }

  vfprintf(stderr, fmt, args);
  va_end(args);

  exit(EXIT_FAILURE);
}
