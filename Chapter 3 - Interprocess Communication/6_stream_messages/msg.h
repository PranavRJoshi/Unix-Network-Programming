#ifndef MSG_H
#define MSG_H

/*
 * Definition of "our" message.
 *
 * You may have to change the 4096 to a smaller value, if message queues on your system were configured with "msgmax" less than 4096
*/

#define   MAXMESGDATA   (4096 - 16)                   /* We don't want sizeof(Mesg) > 4096 */

#define   MESGHDRSIZE   (sizeof(Mesg) - MAXMESGDATA)  /* length of mesg_len and mesg_type */

/*
 * NOTE:  Using the sizeof on Mesg type yields 4096 rather than 4092.
 *        This is becuase structures are padded/aligned making hole.
 *        (Refer to #58 of Chapter 16 of C Programming - A Modern Approach)
 *        Even though an int (on my machine) is of 4 bytes, it is padded/aligned 
 *        with extra 4 bytes (hole usually at the end or between members, never 
 *        at beginning).
*/
typedef struct {
  int   mesg_len;                 /* number of bytes in mesg_data, can be 0 or > 0 */
  long  mesg_type;                /* message type, must be > 0 */
  char  mesg_data[MAXMESGDATA];   /* Actual data */
} Mesg;

/*
 * mesg_send: Send a message by writing on a file descriptor.
 *            The mesg_len, mesg_type, and mesg_data fields must be filled in by the caller.
*/
void mesg_send (int fd, Mesg *mesg_ptr);

/*
 * mesg_recv: Receive a message by reading on a file descriptor.
 *            Fill in the mesg_len, mesg_type and mesg_data fields, and return mesg_len as the return value also.
*/
int mesg_recv (int fd, Mesg *mesg_ptr);

#endif
