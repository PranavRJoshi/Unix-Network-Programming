#include "mesg.h"
#include "err_routine.h"
#include "msgq.h"

/*
 * Send a message using the System V message queue.
 * The mesg_len, mesg_type and mesg_data fields must be filled in by the caller.
*/
void mesg_send (id, mesg_ptr)
int   id;
Mesg  *mesg_ptr; {
  /*
   * Send the message - the type followed by the optional data.
  */
  /*
   * NOTE:  The function declaration of system call msgsnd is:
   *        
   *            int msgsnd(int msqid, struct msgbuf *ptr, int length, int flag);
   *
   *        Returns 0 if the call is successful, else -1 and sets errno.
   *
   *        The programmer is expected to define their own msgbuf like structure
   *        where the `template` of the structure is:
   *
   *        struct msgbuf {
   *          long mtype;         // message type, must be > 0 
   *          char mtext[1];      // message data (flexible array possibly)
   *        }
   *
   *        But, in our case, the Mesg struct has an int member defined at first. 
   *        This creates an issue as the system call expects a struct with long as the first member.
   *        To overcome this, we send the address to the member of type long as the second argument.
   *        Refer to page 141 for more detailed information.
   *
   *        The length argument specifies the number of bytes of the messages. (can be zero)
   *
   *        Lastly, the flag can be IPC_NOWAIT or zero. IPC_NOWAIT value allows the system call to 
   *        return immediately if there is no room on the message queue for the new message.
  */
  if (msgsnd(id, (char *) &(mesg_ptr->mesg_type), mesg_ptr->mesg_len, 0) != 0) {
    err_sys("msgsnd error");
  }
}

/*
 * Receive a message from a System V message queue.
 * The caller must fill in the mesg_type field with the desired type.
 * Return the number of bytes in the data portion of the message.
 * A 0-length data message implies end-of-file.
*/
int mesg_recv (id, mesg_ptr)
int   id;
Mesg  *mesg_ptr; {
  int   n;

  /*
   * Read the first message on the queue of the specified type.
  */
  /*
   * NOTE:  The function declaration of the msgrcv is:
   *            
   *            int msgrcv(int msqid, struct msgbuf *ptr, int length, long msgtype, int flag);
   *
   *        On successful call, msgrcv returns the number of bytes of data in the received message. (does not include 
   *        the long integer message size that is also returned through the ptr argument)
   *
   *        The ptr argument is like the one for msgsnd, and specifies where the received message is stored.
   *
   *        length specifies the size of the data portion of the structure pointed to by ptr. This is the
   *        maximum amount of data that is returned by the system call.
   *
   *        If the MSG_NOERROR bit in the flag argument is set, this speicifies that if the actual data portion 
   *        of the received message is greater than length, just truncate the data portion and return without 
   *        an error.
   *
   *        Not specifying the MSG_NOERROR flag causes an error return if length is not large enough to receive 
   *        the entire message.
   *
   *        For msgtype, 
   *          ->  if msgtype is zero, the first message on the queue is returned. Since each message queue is maintained
   *              as a first-in, first-out list, a msgtype of zero specifies that the oldest message on the queue is to be 
   *              returned.
   *          ->  if msgtype is greater than zero, the first message with a type equal to msgtype is returned.
   *          ->  if msgtype is less than zero, the first message with the lowest type that is less than or equal to the 
   *              absolute value of msgtype is returned.
  */
  n = msgrcv(id, (char *) &(mesg_ptr->mesg_type), MAXMESGDATA, mesg_ptr->mesg_type, 0);

  if ( (mesg_ptr->mesg_len = n) < 0) {
    err_dump("msgrcv error");
  }

  return n;       /* n will be 0 at the end of file */
}
