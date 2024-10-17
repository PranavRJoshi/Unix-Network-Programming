/*
 * Usage:
 *  ->  To prepare the executable, run the `make` command.
 *  ->  Run the executable using `./hello_world`
 *  ->  NOTE: It might seem like the program has some "warnings", but it is legal in C89
 *      For instance, if the function does not have return type specified, like in function
 *      main, it is implicitly assumed to return an int. Also, main does not return anything,
 *      which is not legal in C99, but is legal in C89.
 *      Another thing is, no standard I/O library (stdio) is included, but we have declared 
 *      the function ourself, which is dynamically linked during execution.
 *  ->  To clean the executable, run `make clean`.
*/

/* Explicit function declaration, the compiler includes it later on... */
int printf();

main () {
  printf("Hello, World!\n");
}
