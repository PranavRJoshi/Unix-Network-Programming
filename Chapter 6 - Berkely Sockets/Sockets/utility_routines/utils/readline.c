#include "../common.h"

/*
 * readline:  Read a line from a descriptor. Read the line one byte at a time, looking for the newline.
 *            We store the newline in the buffer, then follow it with a null character (the same as fgets(3)).
 *            We return the number of characters up to, but not including, the null character (same as strlen(3)).
*/

/*
 * C89 function definition format (shown in text):
 *
 *  int readline (fd, ptr, maxlen)
 *  register int   fd;
 *  register char  *ptr;
 *  register int   maxlen; {
 *    ...
 *  }
*/

int readline (register int fd, register char *ptr, register int maxlen) {
  int   n, rc;    /* rc for read_character? used for reading the character using read */
  char  c;

  for (n = 1; n < maxlen; n++) {
    /*
     * Clearly not the best way to use the read system call, as it reads only one byte.
     * According to the text, reading 1 byte at a time requires 10x more CPU time than issuing a 
     * system call for every 10 bytes of data, given we want to read more than 1 byte... duh.
     *
     * Also, since we only read 1 byte, the call must assign 1 to the variable rc.
    */
    if ((rc = read(fd, ptr, 1)) == 1) {       /* read from fd and store in ptr, one byte only */ 
      *ptr++ = c;           /* Simple pointer hack, where the ptr is dereferenced first (post-increment) and assigned the value stored in c, and then ptr is incremented to point to next address--to store next byte/character. */
      if (c == '\n') {      /* If we encounter newline character, stop reading. */
        break;
      }
    } else if (rc == 0) {                     /* read nothing or EOF */                
      if (n == 1) {       /* First iteration, read nothing */
        return 0;
      } else {            /* reached EOF */
        break;
      }
    } else {                    /* read returned < 0. So an error has occurred */
      return -1;          /* error */
    }
  }

  *ptr = 0;       /* Realize that after every successful read, the ptr is advanced to point to next "empty" slot in array. We need not explicitly increment the pointer, and just add the null character. */
  return n;
}
