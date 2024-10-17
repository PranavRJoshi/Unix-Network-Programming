#ifndef MESG_H
#define MESG_H

#define   MAXMESGDATA   (4096 - 16)                   /* We don't want sizeof(Mesg) > 4096 */

#define   MESGHDRSIZE   (sizeof(Mesg) - MAXMESGDATA)  /* length of mesg_len and mesg_type */

typedef struct {
  int   mesg_len;                 /* number of bytes in mesg_data, can be 0 or > 0 */
  long  mesg_type;                /* message type, must be > 0 */
  char  mesg_data[MAXMESGDATA];   /* Actual data */
} Mesg;

void  client    (int msgqid);

void  server    (int msgqid);

void  mesg_send (int id, Mesg *mesg_ptr);

int   mesg_recv (int id, Mesg *mesg_ptr);

#endif
