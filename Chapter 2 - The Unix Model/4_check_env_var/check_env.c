/*
 * Usage:
 *  ->  To prepare the executable, run the `make` command.
 *  ->  Run the program `./check_env`
 *  ->  The program checks if the environment variables: HOME, TERM, and SHELL exists.
 *  ->  To remove the executable, run the `make clean` command.
*/

#include <stdio.h>
/* getenv is a function declared in the standard library <stdlib.h> */
/* char *getenv(const char *); */
/* #include <stdlib.h> */

#define CHECK_ENV(char_ptr, env_str)                    \
if(((char_ptr) = getenv(#env_str)) == (char *) 0) {     \
  printf(#env_str" is not defined\n");                  \
} else {                                                \
  printf(#env_str" = %s\n", char_ptr);                  \
}

main (argc, argv)
int argc;
char **argv; {
  extern char **environ;
  char *env_var_ptr, *getenv(const char *); /* We can only use *getenv() but support for function prototype without parameter is deprecated, so better to be on the safe side. The safest option is to include the standard library <stdlib.h> */
  CHECK_ENV(env_var_ptr, HOME);
  printf("[LOG] The current address stored in env_var_ptr is: %p\n", (void *) env_var_ptr);
  CHECK_ENV(env_var_ptr, TERM);
  printf("[LOG] The current address stored in env_var_ptr is: %p\n", (void *) env_var_ptr);
  CHECK_ENV(env_var_ptr, SHELL);
  printf("[LOG] The current address stored in env_var_ptr is: %p\n", (void *) env_var_ptr);
}
