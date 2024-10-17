#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

/* Format of structure of tag stat in my machine */
/* 
 * typedef for:
 *  dev_t             = __int32_t       (int)
 *  mode_t            = __uint16_t      (unsigned short)
 *  nlink_t           = __uint16_t      (unsigned short)
 *  _darwin_ino64_t   = __uint64_t      (unsigned long long) 
 *  uid_t             = __uint32_t      (unsigned int)
 *  gid_t             = __uint32_t      (unsigned int)
 *  off_t             = __int64_t       (long long)
 *  blkcnt_t          = __int64_t       (long long)
 *  blksize_t         = __int32_t       (int)
 *  
 *
 *  structure:
 *    timespec has members: 
 *      tv_sec        = __darwin_time_t (long)
 *      tv_nsec       = long
 *      */
/*
struct stat __DARWIN_STRUCT_STAT64;

#define __DARWIN_STRUCT_STAT64_TIMES \
  struct timespec st_atimespec;           // time of last access
  struct timespec st_mtimespec;           // time of last data modification
  struct timespec st_ctimespec;           // time of last status change
  struct timespec st_birthtimespec;       // time of file creation(birth)

#define __DARWIN_STRUCT_STAT64 { \
	dev_t                         st_dev;                 // [XSI] ID of device containing file
	mode_t                        st_mode;                // [XSI] Mode of file (see below)
	nlink_t                       st_nlink;               // [XSI] Number of hard links
	__darwin_ino64_t              st_ino;                 // [XSI] File serial number
	uid_t                         st_uid;                 // [XSI] User ID of the file
	gid_t                         st_gid;                 // [XSI] Group ID of the file
	dev_t                         st_rdev;                // [XSI] Device ID
	__DARWIN_STRUCT_STAT64_TIMES
	off_t		                      st_size;                // [XSI] file size, in bytes
	blkcnt_t	                    st_blocks;              // [XSI] blocks allocated for file
	blksize_t	                    st_blksize;             // [XSI] optimal blocksize for I/O
	__uint32_t	                  st_flags;               // user defined flags for file
	__uint32_t	                  st_gen;                 // file generation number
	__int32_t	                    st_lspare;              // RESERVED: DO NOT USE!
	__int64_t	                    st_qspare[2];           // RESERVED: DO NOT USE!
}
*/

main (argc, argv) 
int argc;
char **argv; {
  struct stat *cur_file_stat = NULL;
  int stat_flag;

  cur_file_stat = malloc(sizeof(struct stat));
  if (!cur_file_stat) {
    printf("[ERROR] Failed to allocate sufficient storage to store the stat buffer");
    exit(EXIT_FAILURE);
  }
  /* 
   * initializing cur_file_stat with NULL and passing to stat causes it to return -1 and no stat of the file is provided to the buffer.
  */
  stat_flag = stat(argv[0], cur_file_stat);

  if (stat_flag) {
    printf("[ERROR] Could not read the file.\n");
  } else {
    printf("[SUCCESS] The stats of the file has been stored successfully.\n");
    printf("[LOG] The size of the file (in bytes): %lld\n", cur_file_stat->st_size);
    printf("[LOG] The number of blocks allocated for this program is: %lld\n", cur_file_stat->st_blocks);
    printf("[LOG] The block size is: %d\n", cur_file_stat->st_blksize);
    printf("[LOG] The device ID for the current file is: %d\n", cur_file_stat->st_rdev);
    /* Provides time in the Unix epoch format */
    printf("[LOG] The last access of the file has been (in seconds): %ld\n", (cur_file_stat->st_atimespec).tv_sec);
    printf("[LOG] The user defined flags for the file is: %d\n", cur_file_stat->st_flags);
  }

  free(cur_file_stat);
  cur_file_stat = NULL;
}
