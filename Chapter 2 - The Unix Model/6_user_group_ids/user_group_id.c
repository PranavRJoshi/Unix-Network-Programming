/*
 * Usage: 
 *  ->  To prepare the executable, run the `make` command.
 *  ->  Run the `./user_group_id` program.
 *  ->  To remove the executable, run the `make clean` command.
*/

#include <stdio.h>

unsigned short getuid();  /* real user ID */
unsigned short getgid();  /* real group ID */
unsigned short geteuid(); /* effective user ID */
unsigned short getegid(); /* effective group ID */

main () {
  printf("The real user ID is: %hd\n", getuid());
  printf("The real group ID is: %hd\n", getgid());
  printf("The effective user ID is: %hd\n", geteuid());
  printf("The effective effective group ID is: %hd\n", getegid());
}
