#ifndef MSGQ_H
#define MSGQ_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <errno.h>
extern int errno;

/*
 * The process happens in this manner.
 *  char *path and char proj ---> ftok() ---> key_t key ---> {msg|sem|shm}get() ---> int id
 *
 * We could also use the function ftok:
 *    
 *    key_t ftok (const char *path, int id);
 *
 * The text has a somewhat different function declaration:
 *    
 *    key_t ftok (const char *path, char proj);
 *
 * What it essentailly does is convert a pathname and project identifier into a System V IPC key
 *
 * If the client and server only need a single IPC channel between them (like in this program),
 * a proj of one, say, can be used.
 *
 * If multiple IPC channels are needed, say one from the client to the server, and 
 * another from server to client, then one channel can use a proj of one, and the 
 * other a proj of two, for example.
 *
 * For simplicity purpose, the KEY is hardcoded.
*/
#define   MKEY1   1234

#define   PERMS   0666

#endif
