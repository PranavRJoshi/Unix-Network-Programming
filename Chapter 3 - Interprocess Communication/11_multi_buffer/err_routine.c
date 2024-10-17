#include <stdio.h>
/* #include <varargs.h> */  /* Use stdarg instead. */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "err_routine.h"

char *pname = NULL;

char emesgstr[255] = {0};

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

extern int          errno;                  /* Unix error number */
/*
 * sys_nerr:  Implementation defined number of errors in a system which the global variable errno can be. 
 *            errno variable falls between: errno >= 0 and errno < sys_nerr
*/
extern const int    sys_nerr;               /* Number of error message strings in sys table */
/* 
 * sys_errlist: An array of const (read-only) pointers pointing to const (read-only) object of string.
 *              Standard variable declared in stdio header. 
 *              Contains `sys_nerr` number of strings. 
*/
extern const char   * const sys_errlist[];  /* The system error message table */

#ifdef SYS5
int     t_errno;          /* in case caller is using TLI, these are "tentative definitions"; else they're "definitions" */
int     t_nerr;
char    *t_errlist[1];
#endif

void err_ret (char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (pname != NULL) {
    fprintf(stderr, "%s: ", pname);
  }

  vfprintf(stderr, fmt, args);
  va_end(args);

  my_perror();

  fflush(stdout);
  fflush(stdin);

  return ;
}

/*
 * Fatal error. Print a message, dump core (for debugging) and terminate.
 *
 *      err_dump(str, arg1, arg2, ...);
 *
 * The string "str" must specify the conversion specification for any args.
*/
void err_dump (char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (pname != NULL) {
    fprintf(stderr, "%s: ", pname);
  }

  vfprintf(stderr, fmt, args);
  va_end(args);

  my_perror();

  fflush(stdout);
  fflush(stdin);

  abort();
  exit(EXIT_FAILURE);
}

/*
 * Print the UNIX errno value.
 * We just append it to the end of the emesgstr[] array
*/
void my_perror (void) {
  register int    len;
  char            *sys_err_str();

  len = strlen(emesgstr);
  /* 
   * If the string length in emesgstr is not zero, then start to add the string
   * (the name of the corresponding errno in this case) to the character array
   * after the len 'bytes'
  */
  sprintf(emesgstr + len, " %s", sys_err_str());    
}

/*
 * Return a string containing some additional operating-system dependent information.
 * NOTE that different versions of UNIX assign different meanings to the same value of "errno" 
 * (compare errno's starting with 35 between System V and BSD, for example). 
 *
 * This means that if an error condition is being sent to another UNIX system, we must interpret 
 * the errno value on the system that generated this error, and not just send the decimal value 
 * of errno to the other system.
*/
char *sys_err_str (void) {
  static char msgstr[200];        /* msgstr contains the corresponding errno message. */

  if (errno != 0) {
    if (errno > 0 && errno < sys_nerr) {
      /* msgstr = strerror(errno); */         /* Alternative way, need to declare msgstr as a pointer. strerror returns `const char *` */
      /* strerror_r(errno, (msgstr + 1), 200); */   /* Need to declare msgstr as an array of fixed size. */
      sprintf(msgstr, "(%s)", sys_errlist[errno]);    /* used in text, deprecated as per manual.  */
    } else {
      sprintf(msgstr, "(errno = %d)", errno);
    }
  } else {
    msgstr[0] = '\0';
  }
#ifdef SYS5
  if (t_errno != 0) {
    char  tmsgstr[100];

    if (t_errno > 0 && t_errno < sys_nerr) {
      sprintf(msgstr, " (%s)", t_errlist[t_errno]);
    } else {
      sprintf(msgstr, ", (t_errno = %d)", t_errno);
    }
    strcat(msgstr, tmsgstr);      /* catenate strings */
  }
#endif
  return (msgstr);
}

