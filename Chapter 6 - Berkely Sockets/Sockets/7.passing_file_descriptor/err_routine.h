#ifndef ERR_ROUTINE_H
#define ERR_ROUTINE_H

#include <stdio.h>
/* #include <varargs.h> */  /* Use stdarg instead. */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef CLIENT
#ifdef SERVER
#warning "a program cannot be both server and client simulatenously"
#endif
#endif

#ifndef CLIENT 
#ifndef SERVER
#define CLIENT  1
#endif
#endif 


#ifndef NULL
#define NULL ((void *) 0)
#endif  /* !NULL */

#ifdef CLIENT
void err_sys (char *fmt, ...);

void err_dump (char *fmt, ...);

void err_ret (char *fmt, ...);

void err_quit (char *fmt, ...);

char *sys_err_str (void);

void my_perror (void);
#endif    /* CLIENT */

#ifdef SERVER

void err_init (char *ident);

void err_sys (char *fmt, ...);

void err_dump (char *fmt, ...);

void err_ret (char *fmt, ...);

void err_quit (char *fmt, ...);

char *sys_err_str (void);

void my_perror (void);

#endif    /* SERVER */

char *sys_err_str (void);

#endif
