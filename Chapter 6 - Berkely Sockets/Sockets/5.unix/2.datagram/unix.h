#ifndef UNIX_H
#define UNIX_H

/*
 * Definitions for UNIX domain stream and datagram client/server programs.
*/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdlib.h>   /* for exit() and its macros */
#include <strings.h>  /* for bzero() */
#include <unistd.h>   /* for close() */

#define UNIXSTR_PATH    "./s.unixstr"   /* connection-oriented */
#define UNIXDG_PATH     "./s.unixdg"    /* connectionless */

#define UNIXDG_TMP      "/tmp/dg.XXXXXX"

char *pname;

#endif
