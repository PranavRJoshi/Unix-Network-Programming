#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/errno.h>

/*
 * my_open: Open a file, returning a file descriptor.
 *          
 *          This function is similar to the UNIX open() system call,
 *          however, here we invoke another program to do the actual open(),
 *          to illustrate the passing of open files between processes.
*/
int my_open (char *filename, int mode);


/*
 * s_pipe: Create an unnamed stream socket pair. 
*/
int s_pipe (int fd[static 2]);

/*
 * my_sendfile: Pass a file descriptor to another process.
 *              Return 0 if OK, otherwise return the errno value from the sendmesg() system call.
*/
int my_sendfile (int sockfd, int fd);

/*
 * my_recvfile: Receive a file descriptor from another process.
 *              Return the file descriptor if OK, otherwise return -1.
*/
int my_recvfile (int sockfd);

#endif
