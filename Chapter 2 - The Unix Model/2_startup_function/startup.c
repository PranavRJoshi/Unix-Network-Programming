/*
 * Usage:
 *  ->  Prepare the executable using `make` command*.
 *  ->  Run the program using `./startup`.
 *      The program can take arguments, which will be displayed in order.
 *  ->  To remove the executable, run the `make clean` comand.
*/

/* Without including the standard I/O header, the printf function has undefined behavior when taking the string argument */
#include <stdio.h>

main (argument_count, argument_value)
int argument_count;
char **argument_value; {
  if (argument_count == 1) {
    printf("[LOG] The only argument provided is the function name, which is: %s\n", argument_value[0]);
  } else {
    int count;
    printf("The provided arguments are:\n");
    for (count = 1; count <= argument_count; count++) {
      printf("%d. %s\n", count, argument_value[count - 1]);
    }
  }
  /* return is implicitly implied (C89 only), but it's a good idea to explicity return using the return keyword or exit function */
}
