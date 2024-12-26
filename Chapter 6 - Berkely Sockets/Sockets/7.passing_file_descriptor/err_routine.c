#include "err_routine.h"
#include "systype.h"

char *pname = NULL;

/* NOTE: The va_dcl parameter specified is no longer supported as GCC has stopped the support for varargs.h */
/* Refer to this site: https://pubs.opengroup.org/onlinepubs/7908799/xsh/varargs.h.html */
/*
 *  err_sys (va_alist)
 *  va_dcl
 *  {
 *    ... 
 *  }
*/

#ifdef CLIENT           /* these all output to stderr stream */

/*
 *  VARARGS/STDARGS 1
 *  err_quit: Fatal error. Print a message and terminate. 
 *            Don't dump core and don't print the system's errno value.
 *
 *      err_quit(str, arg1, arg2, ...)
 *
 *  The string "str" must specify the conversion specification for any args.
*/
void err_quit (char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (pname != NULL) {
    fprintf(stderr, "%s: ", pname);
  }

  vfprintf(stderr, fmt, args);
  fputc('\n', stderr);
  va_end(args);

  exit(1);  
}

/*
 *  VARARGS/STDARGS 1
 *  err_sys:  Fatal error related to a system call. Print a message and terminate.
 *            Don't dump core, but do print the system's errno value and its associated message.
 *
 *      err_sys(str, arg1, arg2, ...)
 *
 *  The string "str" must specify the conversion specification for any args.
*/
void err_sys (char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (pname != NULL) {
    fprintf(stderr, "%s: ", pname);
  }

  vfprintf(stderr, fmt, args);
  va_end(args);

  my_perror();

  exit(1);
}

/*
 *  VARARGS/STDARGS 1
 *  err_ret: Recoverable error. Print a message, and return to caller.
 *          
 *      err_ret(str, arg1, arg2, ...)
 *
 *  The string "str" must specify the conversion specification for any args.
*/
void err_ret (char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (pname != NULL) {
    fprintf(stderr, "%s: ", pname);
  }

  vfprintf(stderr, fmt, args);
  va_end(args);

  my_perror();

  fflush(stdin);
  fflush(stdout);

  return;
}

/*
 *  VARARGS/STDARGS 1
 *  err_dump: Fatal error. Print a message, dump core (for debugging) and terminate.
 *
 *      err_dump(str, arg1, arg2, ...)
 *  
 *  The string "str" must specify the conversion specification for any args.
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

  fflush(stdin);        /* abort doesn't flush stdio buffers. */
  fflush(stdout);

  abort();              /* dump core and terminate */
  exit(1);              /* shouldn't get here */
}

/*
 *  my_perror:  Print the UNIX errno value.
*/
void my_perror (void) {
  char *sys_err_str();

  fprintf(stderr, " %s\n", sys_err_str());
}

#endif      /* CLIENT */

#ifdef SERVER

#ifdef BSD 

/*
 * Under BSD, these server routines use the syslog(3) facility.
 * They don't append a newline, for example.
*/

#include <syslog.h>

#else     /* !BSD */

#define   syslog(a,b)       fprintf(stderr, "%s\n", (b))
#define   openlog(a,b,c)    fprintf(stderr, "%s\n", (a))

#endif    /* BSD */

char emesgstr[255] = {0};   /* used by all server routines. */

/*
 *  err_init: Identify ourself, for syslog() messages.
 *
 *            LOG_PID is an option that says prepend each message with out pid.
 *            LOG_CONS is an option that says write to console if unable to send the message to syslogd.
 *            LOG_DAEMON is our facility.
*/
void err_init (char *ident) {
  openlog(ident, (LOG_PID | LOG_CONS), LOG_DAEMON);
}

/*
 *  VARARGS/STDARGS 1
 *  err_quit: Fatal error. Print a message and terminate. 
 *            Don't dump core and don't print the system's errno value.
 *
 *      err_quit(str, arg1, arg2, ...)
 *
 *  The string "str" must specify the conversion specification for any args.
*/
void err_quit (char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vsprintf(emesgstr, fmt, args);
  va_end(args);

  syslog(LOG_ERR, emesgstr);

  exit(1);  
}

/*
 *  VARARGS/STDARGS 1
 *  err_sys:  Fatal error related to a system call. Print a message and terminate.
 *            Don't dump core, but do print the system's errno value and its associated message.
 *
 *      err_sys(str, arg1, arg2, ...)
 *
 *  The string "str" must specify the conversion specification for any args.
*/
void err_sys (char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vsprintf(emesgstr, fmt, args);
  va_end(args);

  my_perror();

  syslog(LOG_ERR, emesgstr);

  exit(1);
}

/*
 *  VARARGS/STDARGS 1
 *  err_ret: Recoverable error. Print a message, and return to caller.
 *          
 *      err_ret(str, arg1, arg2, ...)
 *
 *  The string "str" must specify the conversion specification for any args.
*/
void err_ret (char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vsprintf(emesgstr, fmt, args);
  va_end(args);

  my_perror();

  syslog(LOG_ERR, emesgstr);

  return;
}

/*
 *  VARARGS/STDARGS 1
 *  err_dump: Fatal error. Print a message, dump core (for debugging) and terminate.
 *
 *      err_dump(str, arg1, arg2, ...)
 *  
 *  The string "str" must specify the conversion specification for any args.
*/
void err_dump (char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vsprintf(emesgstr, fmt, args);
  va_end(args);

  my_perror();

  syslog(LOG_ERR, emesgstr);

  abort();              /* dump core and terminate */
  exit(1);              /* shouldn't get here */
}

/*
 *  my_perror:  Print the UNIX errno value.
*/
void my_perror (void) {
  register int  len;
  char          *sys_err_str();

  len = strlen(emesgstr);
  sprintf(emesgstr + len, " %s", sys_err_str());
}

#endif      /* SERVER */

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
