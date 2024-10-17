#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

int main (argc, argv)
int argc;
char **argv; {
  
  int           i;
  struct stat   statbuffer;
  char          *ptr;

  for (i = 1; i < argc; i++) {
    printf("%s: ", argv[i]);
    if (stat(argv[i], &statbuffer) < 0) {
      printf("fstat error\n");
      exit(EXIT_FAILURE);
    }
    /* 
     * No need to go into details of what the operation in the switch expression works for.
     * The main thing to understand is that the expression results in one of the defined way 
     * to determine the type of file.
    */
    switch (statbuffer.st_mode & S_IFMT) {  /* bitwise AND operator for the bitfield manipulation. */
      case S_IFDIR:         ptr = "directory";              break;
      case S_IFCHR:         ptr = "character special";      break;
      case S_IFBLK:         ptr = "block special";          break;
      case S_IFREG:         ptr = "regular";                break;
      #ifdef S_IFLNK 
      case S_IFLNK:         ptr = "symbolic link";          break;
      #endif 
      #ifdef S_IFSOCK
      case S_IFSOCK:        ptr = "socket";                 break;
      #endif 
      #ifdef S_IFIFO 
      case S_IFIFO:         ptr = "fifo";                   break;
      #endif 
      default:              ptr = "** unknown mode **";     break;
    }
    printf("%s\n", ptr);
  }

  return 0;
}
