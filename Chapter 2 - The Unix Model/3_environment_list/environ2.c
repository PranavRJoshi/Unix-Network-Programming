#include <stdio.h>

main (argc, argv)
int argc;
char **argv; {
  int env_count;

  if (argc == 1) {
    printf("[LOG] No additional arguments provided to the main process.\n");
  } else {
    int count;
    printf("[LOG] The provided arguments are:\n");
    for (count = 1; count <= argc; count++) {
      printf("%d. %s\n", count, argv[count - 1]);
    }
  }
  printf("[LOG] The environment variables are: \n");
  /* Although **variable_name and *variable_name[] are similar, apparently, environ should be declared as the one specified below, using the other one induced undefined behavior. */
  extern char **environ;
  for (env_count = 0; environ[env_count] != (char *) 0; env_count++) {
    printf("%d. %s\n", env_count, environ[env_count]);
  }
}

