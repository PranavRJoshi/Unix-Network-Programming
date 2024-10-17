#include "msg.h"
#include "err_routine.h"

#include <unistd.h>

/*
 * Functionality: The length of message is determined (along with the header size), and is then
 *                written to the provided file descriptor (in bytes). The function is used to 
 *                send the data to the pipe (named pipe in our case). 
 *                
 *                NOTE: The write call may write bytes value (or even unspecified holes in structures)
 *                but a structure that can store the value in similar structure can receive and store 
 *                the correct value. (check the test directory)
 *                (For instance, in this struct, the first two members are int and long, so
 *                when the values of int and long are written in a file/pipe, it can be "unreadable", 
 *                but after reading the bytes from the file/pipe and storing in the same struct, the
 *                corresponding bytes value will be used by the type (int and long).)
*/
void mesg_send (fd, mesg_ptr)
int   fd;
Mesg  *mesg_ptr; {
  int n;

  /*
   * Write the message header and the optional data.
   * First calculate the length of the length field, plus the type field, plus the optional data.
  */
  n = MESGHDRSIZE + mesg_ptr->mesg_len;

  if (write(fd, (char *) mesg_ptr, n) != n) {
    err_sys("message write error");
  }
}

/*
 * Functionality: The function is called to read the pipe to check for bytes of size 
 *                MESGHDRSIZE (16 in our case). It is stored in the pointer to the 
 *                structure mesg_ptr parameter. If the data is actual header of the 
 *                message envelope, the size of the message is read from the structure
 *                and the respective size is read from the file descriptor (pipe in our
 *                case).
 *
 *                NOTE: The function might be called with a valid envelope that contains 
 *                the mesg_len field as zero (0). Such envelope is send by the process 
 *                which indicates the envelope content is fulfilled.
*/
int mesg_recv (fd, mesg_ptr)
int   fd;
Mesg  *mesg_ptr; {
  int n;

  /*
   * Read the message header first. This tells us how much data follows the message for the next read.
   * An end-of-file on the file descriptor causes us to return 0.
   * Hence, we force the assumption on the caller that a 0-length message means EOF.
  */
  if ( (n = read(fd, (char *) mesg_ptr, MESGHDRSIZE)) == 0) {
    return (0);       /* end of file */
  } else if (n != MESGHDRSIZE) {
    err_sys("message header read error");
  }

  /*
   * Read the actual data, if there is any.
  */
  if ( (n = mesg_ptr->mesg_len) > 0) {
    if (read(fd, mesg_ptr->mesg_data, n) != n) {
      err_sys("data read error");
    }
  }

  return (n);
}
