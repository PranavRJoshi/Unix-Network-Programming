#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef unsigned int my_size_t;

struct my_iovec {
  void            *address;
  my_size_t       buff_length;
};

/*
 * What we could do is: 
 *    - Either `write` each individual buffer to the file descriptor.
 *    - Make a buffer that is large enough to hold the data from all the `my_iovec` structure elements, and call the `write` once.
 *
 * Here, I will use the latter one.
 *
 * Also, one more thing to note is that the `writev (2)` system call is an atomic operation. According to the test:
 *      
 *      ```
 *      The `writev` system call is an atomic operation. This is important if the descriptor refers to a record-based entity,
 *      such as a datagram socket or a magnetic tape drive. For example, a `writev` to a datagram socket produces a single 
 *      datagram, whereas multiple `write`s would produce multiple datagrams. The only correct way to emulate `writev` is 
 *      to copy all the data into a single buffer and then execute a single `write` system call.
 *      ```
 *
 * Thankfully, the implementaion I wrote does this as well. Picking the latter choice turned out to be fruitful.
 * Regarding atomicity, as the function first allocates a buffer capable to hold content from each provided 
 * element in `my_iov`, and then attempts to `write` it in a single call, it can:
 *    ->  write all the content
 *    ->  write only some content
 *    ->  write nothing (errno is set).
 * For the call of nature `n = write(fd, buf, size)`, the return from `write`--the total bytes written to fd--will be assigned 
 * to `n`. If `n` equals `size`, it means all the content from `buf` was written to `fd`. There also exists cases when `n` is 
 * less than `size`, which indicates not all content was written.
 * The function below asserts that `n` equal to `size`. If not, the allocated memory is `free`d and the buffer is made to point to NULL 
 * before exiting (maybe returning 0 or -1 would be better). 
*/
my_size_t my_writev (int fd, const struct my_iovec *my_iov, int my_iov_cnt) {
  /*
   * Determine the total space required.
  */
  my_size_t total_buffer_size = 0;
  for (int i = 0; i < my_iov_cnt; i++) {
    total_buffer_size += my_iov[i].buff_length;
  }

  fprintf(stdout, "[LOG] The total size of the buffer is: %u\n", total_buffer_size);

  /*
   * Allocate a buffer with the size required. 
  */
  char *contiguous_buff = malloc(total_buffer_size);
  if (contiguous_buff == NULL) {
    fprintf(stderr, "[malloc error] Failed to initialize the buffer to store all iovec's data.\n");
    exit(EXIT_FAILURE);
  }
  
  /* copy each `my_iovec` element's data to the large buffer */
  int seek = 0;
  for (int i = 0; i < my_iov_cnt; i++) {
    memcpy(contiguous_buff + seek, (char *) my_iov[i].address, my_iov[i].buff_length);
    seek += my_iov[i].buff_length;
  }

  /* use one write system call to write everything */
  my_size_t write_ret = 0;
  if ( (write_ret =  write(fd, contiguous_buff, total_buffer_size)) != total_buffer_size) {
    fprintf(stderr, "[write error] Failed to write everything to the given descriptor.\n");
    /* error occured, so free the malloc'd buffer and exit */
    free(contiguous_buff);
    contiguous_buff = NULL;
    exit(EXIT_FAILURE);
  }

  /* free the buffer */
  free(contiguous_buff);
  contiguous_buff = NULL;     /* won't probably have a case of "dangling" pointer, but it's always good to take precaution :) */

  /* return the total bytes written to fd. */
  return (write_ret);
}

int main (void) {

  char test1[100] = "Hello, ";
  char test2[200] = "myiov!";

  /*
   * So here's the thing. the `sizeof` operator returns the size of an lvalue. In our case, although the character array is 
   * initialized with a string literal of size `n`--where `n` is less than the size of the character array--the `sizeof` for, 
   * say test1, is 100, even though it contains the string literal of `strlen` 7. 
   *
   * Now, one might wonder what the other bytes are holding, since the character array stores no more than 7 (or 8 including the 
   * null character) characters. Well, according to the C99/C11 standard, Section 6.7.8/6.7.9 `Constraints` # 14, it states:
   *
   *      ``` C11
   *      An array of character type may be initialized by a character string literal or UTF−8 string literal, 
   *      optionally enclosed in braces. Successive bytes of the string literal (including the terminating null character 
   *      if there is room or if the array is of unknown size) initialize the elements of the array. 
   *      ```
   *
   *      ``` C99
   *      An array of character type may be initialized by a character string literal, optionally enclosed in braces. 
   *      Successive characters of the character string literal (including the terminating null character if there is 
   *      room or if the array is of unknown size) initialize the elements of the array.
   *      ```
   *
   * NOTE: Altough the standards defined by ISO are not free, the drafts are. The one I took reference from is linked below:
   *   [C11] link - https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf
   *   [C11] page - 158
   *   [C99] link - https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf
   *   [C99] page - 138
   * 
   * So, even if we set the `buff_length` using the `sizeof` operator, we can still ensure no undefined behavior is witnessed.
   * This is because other memory field is filled with zeroes.
   *
   * Also, assignments differ from initialization. If we were to assign a string literal or any C-string using library function
   * such as family of `strcpy` function, we need to make sure how the system handles the remaining bytes. To be on the safer side,
   * we can define our own `strcpy` like function, the one which I wrote looks something like this: 
   *
   *    ```
   *    int my_strlcpy (char *dest, const char *src, int d_len) {
   *      int d_i = 0;
   *      while (*src != '\0') {
   *        if (d_i < d_len) {
   *          *(dest + d_i) = *src;
   *          d_i++;
   *        }
   *        src++;
   *      }
   *      *(dest + d_i) = '\0';
   *      return d_i;
   *    }
   *    ```
   * 
   * What we can do here is, add another parameter to the function which specifies the size of the `dest` character array. If the 
   * array is not filled completely, we not only append one null character to the array `dest`, but all other remaining elements 
   * with the null character.
   *
   * We know that the `write` system call writes to the descriptor provided as the argument. For the given function prototype:
   *
   *    ```
   *    ssize_t write(int fildes, const void *buf, size_t nbytes);
   *    ```
   * 
   * `write` function writes `nbytes` from the buffer `buf` to the file descriptor `fildes`. In this scenario, even if the `write` 
   * system call is required to write hundreds of bytes (300 to be precise in this example), the bytes which holds the null character
   * or zeroes are technically not "written", as it writes nothing. So the program functions as intended with no undefined behavior.
   *
   * But it is still a waste of computation as we are still "writing", even though it's nothing, so to optimize the call, one may set 
   * the `buff_length` using the `strlen` function.
  */
  
  struct my_iovec my_iovec_test[2];

  my_iovec_test[0].address = test1;
  my_iovec_test[0].buff_length = sizeof(test1);

  my_iovec_test[1].address = test2;
  my_iovec_test[1].buff_length = sizeof(test2);

  my_writev(1, &my_iovec_test[0], 2);

  return 0;
}
