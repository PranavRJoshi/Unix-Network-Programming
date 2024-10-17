#ifndef FIFO_H
#define FIFO_H

#define   MAXBUFF   1024
#define   FIFO1     "/tmp/fifo.1"
#define   FIFO2     "/tmp/fifo.2"
#define   PERMS     0666

/*
 * client
 * args:          readfd  - read from the FIFO2
 *                writefd - write to the FIFO1
 * functionality: Reads from the standard input, stores the content in buff. Reads atmost MAXBUFF characters to the buff.
 *                The read string is then written to the writefd, which is the writing end of pipe1.
 *                After reading the filename, it is sent to the reading end of pipe1, which is read by server function in server.c
 *                While the input from the pipe2 is buffered by server, the client reads it, and sends the buffer to the standard output.
*/
void client (int readfd, int writefd);

/*
 * server
 * args:          readfd  - read from the FIFO1
 *                writefd - write to the FIFO2
 * functionality: Reads from reading end of FIFO1, stores the content in buff. Reads atmost MAXBUFF characters to the buff.
 *                After reading the filename, the file is opened (in read-only) and checked if any error occured. Appropriate 
 *                error message is printed using the sys_err_str--based on the errno. Upon successful call, the fd is now a 
 *                file descriptor to the respective file, and then contents of the file is read at most MAXBUFF and stored to 
 *                the buff array. Recall that read automatically adjusts the offset after each call, so we don't have to manually 
 *                configure the byte offset to the file.
 *                The read item is then buffered to the writefd (writing end of FIFO2 which is read by client) in each read call.
 *                After encountering the EOF, the read operation sets the byte-offset to the end of the file such that the next call 
 *                will return a zero. Hence, we know when to terminate reading the file.
 *                The final if statement signifies that read operation was not successful.
*/
void server (int readfd, int writefd);

#endif
