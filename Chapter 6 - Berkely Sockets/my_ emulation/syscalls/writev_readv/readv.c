#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>

typedef unsigned int my_size_t;

struct my_iovec {
  void            *address;
  my_size_t       buff_length;
};

/*
 * According to the text, "`readv` always fills one buffer (specified by iov_len, or buff_length in our case) before 
 * proceeding to the next buffer in the `iov` array (`my_iov` in our case)."
 *
 * Also, the function assumes that each element of `my_iov` contains `address` member which can hold `buff_length` bytes of data.
*/
my_size_t my_readv (int fd, const struct my_iovec *my_iov, int my_iov_cnt) {
  /*
   * Determine the total space required.
  */
  my_size_t total_buffer_size = 0;
  for (int i = 0; i < my_iov_cnt; i++) {
    total_buffer_size += my_iov[i].buff_length;
  }
  /*
   * Allocate a buffer with the size required. 
  */
  char *contiguous_buff = malloc(sizeof(total_buffer_size));
  if (contiguous_buff == NULL) {
    fprintf(stderr, "[malloc error] Failed to initialize the buffer to store all iovec's data.\n");
    exit(EXIT_FAILURE);
  }

  my_size_t read_ret = 0, read_seek = 0;
  my_size_t read_left = total_buffer_size;
  for (;;) {
    read_ret = read(fd, contiguous_buff + read_seek, read_left);
    if (read_ret < 0) {
      fprintf(stderr, "[read error] Failed to read from the given descriptor.\n");
      /* error occured, so free the malloc'd buffer and exit */
      free(contiguous_buff);
      contiguous_buff = NULL;
      exit(EXIT_FAILURE);
    }
    if (read_ret == 0 && read_left > 0) {   /* cannot read more, yet the buffer specified is not filled completely */
      fprintf(stderr, "[read error] reached the end of file but buffer is not full.\n");
      /* error occured, so free the malloc'd buffer and exit */
      free(contiguous_buff);
      contiguous_buff = NULL;
      exit(EXIT_FAILURE);
    }
    read_seek += read_ret;
    read_left -= read_ret;
    if (read_left == 0) {
      break;
    }
  }

  // if ( (read_ret = read(fd, contiguous_buff, total_buffer_size)) != total_buffer_size) {
  //   fprintf(stderr, "[read error] Failed to read everything from the given descriptor.\n");
  //   /* error occured, so free the malloc'd buffer and exit */
  //   free(contiguous_buff);
  //   contiguous_buff = NULL;
  //   exit(EXIT_FAILURE);
  // }

  /*
   * For some weird reason, using `fflush (3)` or `fpurge (3)` won't work on `stdin`, and the remaining buffer is 
   * used as the input for the next terminal command.
  */
  if (fd == 0) {                  /* incase the descriptor is standard input (stdin), flush remaining data */
    while (getchar() != '\n') {
      ;
    }
  }

  int seek = 0;
  for (int i = 0; i < my_iov_cnt; i++) {
    memcpy((char *) my_iov[i].address, contiguous_buff + seek, my_iov[i].buff_length);
    seek += my_iov[i].buff_length;
  }

  /* free the buffer */
  free(contiguous_buff);
  contiguous_buff = NULL;     /* won't probably have a case of "dangling" pointer, but it's always good to take precaution :) */

  /* return the total bytes written to fd. */
  return (read_seek);
}

int main (void) {
  
  char first_name[10];
  char last_name[10];
  struct my_iovec test_vec[2];

  test_vec[0].address = first_name;
  test_vec[0].buff_length = sizeof(first_name);

  test_vec[1].address = last_name;
  test_vec[1].buff_length = sizeof(last_name);

  int ret = 0;
  
  /* test for file descriptor other than standard input (stdin) */
  // int testfd;
  //
  // if ((testfd = open("./insufficient", O_RDONLY)) < 0) {
  //   fprintf(stderr, "[open error] failed to open a file.\n");
  //   exit(EXIT_FAILURE);
  // }
  //
  // ret = my_readv(testfd, test_vec, 2);

  // or
  
  /* test for standard input */
  ret = my_readv(0, test_vec, 2);

  fprintf(stdout, "[LOG] return value of me_readv: %d\n", ret);
  fprintf(stdout, "[LOG] Not-C-String stored in first element is: \n");
  write(1, test_vec[0].address, test_vec[0].buff_length);
  fprintf(stdout, "\n");
  fprintf(stdout, "[LOG] Not-C-String stored in second element is: \n");
  write(1, test_vec[1].address, test_vec[1].buff_length);
  fprintf(stdout, "\n");

  exit(EXIT_SUCCESS);
}
