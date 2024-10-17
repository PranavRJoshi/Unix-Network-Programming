#ifndef ERR_ROUTINE_H
#define ERR_ROUTINE_H

#ifdef CLIENT
#ifdef SERVER
/* can't define both CLIENT and SERVER */
#endif  /* SERVER */
#endif  /* CLIENT */

#ifndef CLIENT
#ifndef SERVER
#define CLIENT  1
#endif  /* !SERVER */
#endif  /* !CLIENT */

#ifndef NULL
#define NULL ((void *) 0)
#endif  /* !NULL */

void err_sys (char *fmt, ...);

char *sys_err_str (void);

#endif
