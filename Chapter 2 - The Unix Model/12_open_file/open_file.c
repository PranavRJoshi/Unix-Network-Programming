/*
 * Usage: 
 *  ->  Prepare the executable using the `make` command in the current working directory.
 *  ->  execute `./open_file`
 *  ->  This should create a new file called "clone_file.c" which will have the contents of open_file.c
 *  ->  To clean, run `make clean`
*/

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <sys/stat.h>

/* 
 *  umask from the man (2) page:
 *  The umask() routine sets the process's file mode creation mask to cmask and returns the previous value of the mask. 
 *  The 9 low-order access permission bits of cmask are used by system calls, including 
 *    open(2), 
 *    mkdir(2), 
 *    mkfifo(2), and 
 *    mknod(2) 
 *  to turn off corresponding bits requested in file mode. 
 *  (See chmod(2)). 
 *  This clearing allows each user to restrict the default access to his files. 
 *
 *  The default mask value is S_IWGRP | S_IWOTH (022, write access for the owner only). 
 *  Child processes inherit the mask of the calling process.
*/

int main (void) {

  int src_file_descriptor;
  int clone_file_descriptor;

  char read_buf[128];
  int check_read_flags;
  int check_write_flags;

  int total_bytes_stored = 0;

  int stat_flag;
  struct stat written_file_stat;

  if ((src_file_descriptor = open("./open_file.c", O_RDONLY)) != -1) {
    /* When umask command is entered in the terminal, it's usually 022 (higher order digit: user r/w/x access, second digit: group r/w/x, and the lower order digit: other r/w/x). When using the creat sys call to create a file, the second parameter--the file mode--is subtracted with the umask, so if the mode in creat is 0777 (leading zero just indicates that the value provided is in base-8/octal representation), it will be subtracted by 022. The result is 0755, which means that the group and other cannot write in the file. Using the umask(0) clears out the mask and lets the program create the file with file mode 0777. */
    /*
    mode_t old_mask = umask(0);                 // Sets the umask to 0.
    printf("The old mask is: %o\n", old_mask);  // Returns the previous umask, usually 022
    umask(old_mask);                            // Sets the umask to the initial one, making it 022.
    */
    clone_file_descriptor = creat("./clone_file.c", 0666);    /* 9 bits/digit in binary (3 octal digits, each octal digit takes 3 bits) are used to represent the file mode. The format is read(r)/write(w)/execute(x) for--from higher order to lower order--user, group and others respectively. */
    stat_flag = fstat(clone_file_descriptor, &written_file_stat);
    printf("[STAT LOG] The mode of the file is: %o\n", written_file_stat.st_mode);  /* Only the last 3 digits (in octal form) are significant. For instance, if the last 3 digits are 644, then the mode is: rw(110) for users, r(100) for group, and r(100) for others. */
    /* 
     * One may wonder that the file being read will read from the starting byte position of the file. This is because, we haven't explicitly mentioned the current byte position (or the offset) and just read from the file. 
     * The neat trick used here is the fact that the current byte position is updated after every read call, updating the current byte position with the offset of bytes read and returned in `check_read_flags`. I think this means that, at first, the current byte position is 0 (the first character in the first line) and say that 128 bytes have been read. Now the second call of read will have the current byte position of (0 + 128) bytes. So we don't need to explicitly update the bytes position.
    */
    do {
      if ((check_read_flags = read(src_file_descriptor, read_buf, 128)) <= 128) {
        printf("[READ LOG] Total bytes read is: %d\n", check_read_flags);
        /* only write `check_read_flags` bytes in the file */
        check_write_flags = write(clone_file_descriptor, read_buf, check_read_flags);
        printf("[WRTIE LOG] Total bytes written is: %d\n", check_write_flags);
      }
      total_bytes_stored += check_read_flags;
    } while (check_read_flags != 0);
    /* check_read_flags == 0 when the read sys call encounters the End-Of-File (EOF) in the file */
  }
  /* One thing I noticed is that, when the EOF is encountered, the next call of read will return 0, assuming that the bytes position is pointing to the EOF. */
  printf("[LOG] The total bytes read and written is: %d\n", total_bytes_stored);

  return 0;
}
