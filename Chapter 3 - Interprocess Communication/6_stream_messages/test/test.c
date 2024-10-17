#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/fcntl.h>

#define   MAXMESGDATA   (4096 - 16)
#define   MESGHDRSIZE   (sizeof(Mesg) - MAXMESGDATA)    /* sizeof(Mesg) will be 4096 */
#define   FILENAME      "temp"

typedef struct {
  int   mesg_len;
  long  mesg_type;
  char  mesg_data[MAXMESGDATA];
} Mesg;

int main (void) {
  Mesg msg, msg_recv;
  int filefd;
  int n;

  filefd = open(FILENAME, O_RDWR);

  msg.mesg_type   = 100L;
  strcpy(msg.mesg_data, "This is a conversion of a struct pointer to a character pointer.");  /* length of string = 64 */

  msg.mesg_len = strlen(msg.mesg_data);

  /*
   * Write the contents of the msg variable to the filefd file descriptor.
   * The total number of bytes written will be as the third argument.
  */
  write(filefd, (char *) &msg, MESGHDRSIZE + msg.mesg_len);

  /* reset the file offset. */
  lseek(filefd, 0L, 0);

  /* 
   * Read MESGHDRSIZE bytes of data from the filefd file descriptor and store the contents 
   * in the variable msg_recv variable.
  */
  n = read(filefd, (char *) &msg_recv, MESGHDRSIZE);

  if (n != MESGHDRSIZE) {
    printf("Something is wrong.\n");
  } else {
    printf("Nothing is wrong.\n");
  }

  printf("The returned value is: %d and %ld\n", msg_recv.mesg_len, msg_recv.mesg_type);

  return 0;
}

