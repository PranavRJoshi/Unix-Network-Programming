#include <stdio.h>

/* Recall that **variable_name and *variable_name[] are identical. The former one is stated as a "pointer to a pointer to char" and the latter one is stated as an "array of pointer to char"*/
main (argc, argv, envp)
int argc;
char **argv;
char *envp[]; {
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
  /* printf("The address where null pointer of type (char *) is: %p\n", (void *) (char *) 0); */
  /* produces 0x0 in my machine */
  /* (char *) is "guaranteed" to produce a null pointer of type (char *) */
  for (env_count = 0; envp[env_count] != (char *) 0; env_count++) {
    printf("%d. %s\n", env_count, envp[env_count]);
  }
}
