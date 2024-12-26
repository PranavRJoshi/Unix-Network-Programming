/*
 * Exercise 6.6: Write a program that copies standard input to standard output using the select system call.
*/

#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFSIZE    4096

int main (void) {

  fd_set read_set, write_set;     /* read and write file descriptor sets which we're interested in selecting for further operation. */

  FD_ZERO(&read_set);       /* Clear the bits to prevent undefined behavior as the set objects are automatic variable. */
  FD_ZERO(&write_set);

  FD_SET(0, &read_set);     /* Set the standard input as one of member of read file descriptor set used in `select` */
  /*
   * This one is a bit tricky. We aren't really obliged to set the member for write file descriptor set prematurely 
   * as we first intend to read the data and then only write it to the standard output. The code does "toggle" between 
   * the set using the `FD_CLR` and `FD_SET` macros inside the loop to take turn.
   *
   * Also, do note that we ought to initialize `buffer` array with `{ 0 }` and initialize `read_bytes` as `0`. This is 
   * because when we set both `stdin` and `stdout` descriptor for the argument for select, for some reason, the select 
   * returns with the `stdout` descriptor being the first one ready to write to the standard output. We don't want that 
   * as first the data must be read from the standard input before it is written to the standard output.
   *
   * As `read_bytes` is an automatic variable, it will have garbage value unless initialized with a desired value.
  */
  FD_SET(1, &write_set);

  char  buffer[BUFFSIZE]  = { 0 };
  int   read_bytes        = 0;
  int   select_ret        = 0;
  int   stop_reading      = 0;


  for (;;) {
    /*
     * Wait indefinitely till one of the buffer is ready.
     * It does not wait for multiple buffers tho, since we are "toggling" the descriptors at the end 
     * of the loop below.
     *
     * Also, since the second, third, and fourth arguments are `value-result` argument, after the call returns,
     * the object `read_set` and `write_set` will be modified and will contain the descriptors which are ready.
    */
    if ( (select_ret = select(2, &read_set, &write_set, (fd_set *) 0, (struct timeval *) 0)) < 0) {
      perror("select error: failed to select a file descriptor from the set");
      return (-1);
    }

    // fprintf(stdout, "log: select returned %d\n", select_ret);

    /* 
     * Check if the read descriptor (stdin descriptor) is ready for I/O. 
     * If it's ready, start reading from the standard input and store at-most `BUFFSIZE` bytes in the 
     * variable `buffer`.
     * Also set `stop_reading` to indicate that reading has been completed.
    */
    if (FD_ISSET(0, &read_set)) {
      // fprintf(stderr, "log: reading...\n");
      if ((read_bytes = read(0, buffer, BUFFSIZE)) < 0) {
        perror("read error");
        return (-1);
      }
      if (read_bytes == 0) {
        fprintf(stderr, "read log: has nothing left to read, Exiting...");
        return (0);
      }
      stop_reading = 1;
    }

    /*
     * Check if the write descriptor (stdout descriptor) is ready for I/O.
     * I have encountered this is the first one to be ready, between `stdin` and `stdout`.
     * Then, write at-most `read_bytes` bytes of data from `buffer` to the standard output (1).
     * Also reset `stop_reading` to indicate that reading can be continued.
    */
    if (FD_ISSET(1, &write_set)) {
      // fprintf(stderr, "log: writing...\n");
      if (write(1, buffer, read_bytes) != read_bytes) {
        perror("write error");
        return (-1);
      }
      stop_reading = 0;
    }

    /*
     * If stop_reading is set, the content from stdin has been read and stored in a buffer. 
     * Hence we clear the descriptor from `read_set` and set the descriptor for `write_set`
     * to indicate the process will now want to write to the standard output.
     *
     * If stop_reading is reset, the content received from stdin has been written to the 
     * stdout. Now, we wish to read again from the standard input, hence it is set. Also, we 
     * need to reset the descriptor from `write_set` to indicate that we do not wish to write 
     * at the moment.
    */
    if (stop_reading) {
      FD_CLR(0, &read_set);
      FD_SET(1, &write_set);
    } else {
      FD_CLR(1, &write_set);
      FD_SET(0, &read_set);
    }
  }

  return (0);
}
