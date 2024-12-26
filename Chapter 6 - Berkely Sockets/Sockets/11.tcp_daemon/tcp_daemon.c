#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/ttycom.h>
#include <sys/fcntl.h>

#include <errno.h>
extern int errno;

#ifdef SIGTSTP               /* true if BSD system */
  #include <sys/file.h>
  #include <sys/ioctl.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>

#include "utils.h"

#define MSG_LEN 256

/*
 * SIGCHLD: Signal is sent to the parent process when a child process terminates. Typically, it is discarded if the process 
 *          does not catch it. For 4.3BSD, this signal also indicates that the status of a child process has changed. This is 
 *          more general than just indicating the death of a child process. The change in status can be the death of a child 
 *          process, or it can be that a child process is stopped by a SIGSTOP, SIGTTIN, SIGTTOU, or SIGTSTP signal.
*/
void sig_child        (int sig_id);
void daemon_start     (int ignore_sig_child);

/*
 * TODO:
 * What I really want to do is, after getting the "QUIT COMMAND" from the client, the server will log into the 
 * log file used below. This may lead to race condition if there are multiple clients connected to the server 
 * and most of them quits at the same time. Also, other forms of error are also written to the log file.
 * I think what can be done is, for each connection, use the `mktemp` call to make a temporary file name, and those 
 * logs will be written to the respective log files. Once the client wishes to quit, the server will use 
 * semapohores to enter into the critical section (the one master log file), and copies all the content of the 
 * temporary log file into it as well as deleting the temporary log file. 
*/

/* hard-coded entry point for a daemon */
int main (void) {

  printf("Initiating the daemon...\n");
  daemon_start(1);

  return 0;
}

void sig_child (int sig_id) {
  int pid;
  int status;

  /*
   * WNOHANG is speicified, so, the call is non-blocking. 
   * Kinda confusing as the manual specifes that this call will return the pid of 
   * child process that terminated, which will be clearly greater than 0. And this signal is 
   * sent only when the child process terminates.
  */
  while ((pid = wait3(&status, WNOHANG, (struct rusage *) 0)) > 0) {
    ;
  }
}

void daemon_start (int ignore_sig_child) {
  register int childpid, fd, log_fd;

  if (getppid() == 1) {
    goto out;
  }

  /*
   * Ignore the terminal stop signals (BSD)
  */
  #ifdef SIGTTOU    /* Signal which is generated when background process attempts to write to its control terminal, ignored. */
    signal(SIGTTOU, SIG_IGN);
  #endif  /* SIGTTOU */
  #ifdef SIGTTIN    /* Signal which is generated when background process attempts to read from its control terminal, ignored. */
    signal(SIGTTIN, SIG_IGN);
  #endif  /* SIGTTIN */
  #ifdef SIGTSTP    /* Signal which is sent to a process when the suspend key (CTRL + Z) or delayed suspend key (CTRL + Y) is entered  */
    signal(SIGTSTP, SIG_IGN);
  #endif  /* SIGTSTP */

  if ( (childpid = fork()) < 0 ) {
    perror("can't fork first child.");
    return;
  } else if (childpid > 0) {
    exit(0);      /* parent */
  }

  #ifdef SIGTSTP    /* BSD */
    if (setpgid(0, getpid()) == -1) {
      perror("can't change process group.");
      return;
    }
    /*
     * README
     * The /dev/tty file is a special file that represents the controlling terminal of the process. 
     * When a process opens /dev/tty, it's essentially accessing its own controlling terminal.
     * Try this in a your terminal emulator:  `pwd > /dev/tty`
     *
     * For more information about this special file, read the QnA given below:
     * https://stackoverflow.com/questions/8514735/what-is-special-about-dev-tty
     *
     * For some history on TTYs, check the link below:
     * https://www.linusakesson.net/programming/tty/
    */
    if ((fd = open("/dev/tty", O_RDWR)) >= 0) {
      ioctl(fd, TIOCNOTTY, (char *) NULL);    /* lose controlling tty (tty = tele-type) */
      close(fd);
    }
  #else             /* System V */
    if (setpgrp() == 1) {
      err_sys("can't change process group");
    }
    signal(SIGHUP, SIG_IGN);    /* immune from pgrp leader death */
    if ((childpid = fork()) < 0) {
      err_sys("can't fork second child");
    } else if (childpid > 0) {
      exit(0);      /* first child */
    }

    /* second child */
  #endif 

  out:
    for (fd = 0; fd < NOFILE; fd++) {
      close(fd);
    }

    errno = 0;      /* probably got set to EBADF from a close. */

    chdir("/");

    umask(0);

    if (ignore_sig_child) {
      #ifdef SIGTSTP
        signal(SIGCHLD, sig_child);
      #else 
        signal(SIGCLD, SIG_IGN);        /* System V */
      #endif  /* SIGTSTP */
    }

  /* ************************************************************************************************************** *
   *                                        DAEMON PROCESS STARTS BELOW                                             *
   * ************************************************************************************************************** * */

  /*
   * SOME NOTE BEFORE YOU CHECK THE CODE:
   * The question (Exercise 6.1) specifies that we **call** daemon_start function from one of the server mentioned 
   * in Section 6.6. Obviously, this program does most of the work *after* the daemon has been initiated. 
  */

  /* Open up the log file to write some logs, as we won't have the std{in|out|err} stream */
  const char *log_path = "./tmp/tcp_daemon_log.txt";

  if ( (log_fd = open(log_path, O_RDWR | O_APPEND | O_CREAT, 0666)) < 0 ) {
    /* not like this will print the error, the stderr stream has already been closed. */
    perror("open error: failed to open or create a log file");    
    return;
  }

  /* 
   * Not really needed, just wanted to showcase that we can use this to get the lowest descriptor (which I got was 1, the one 
   * for standard output) as we have previously closed all other descriptor.
  */
  int new_log_fd = dup(log_fd);
  
  int                 sockfd;
  int                 accepted_sock_fd;
  int                 client_sockaddr_size = sizeof(struct sockaddr_in);
  int                 concurrent_child_fd;
  struct sockaddr_in  any_addr, client_addr;

  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket error: failed to create a socket");
  }

  any_addr.sin_family       = AF_INET;
  any_addr.sin_port         = htons(6969);
  any_addr.sin_addr.s_addr  = htonl(INADDR_ANY);

  if ( bind(sockfd, (struct sockaddr *) &any_addr, sizeof(any_addr)) < 0) {
    perror("bind error: failed to bind well known address with port 6969.");
  }

  if ( listen(sockfd, 5) < 0) {
    write_log(new_log_fd, "listen error: failed to listen to 5 concurrent sockets for the speicifed Internet address and port\n");
    return;
  }

  for (;;) {
    if ( (accepted_sock_fd = accept(sockfd, (struct sockaddr *) &client_addr, (socklen_t *) &client_sockaddr_size)) < 0) {
      write_log(new_log_fd, "accept error: failed to accept any connection\n"); 
      return;
    } else {
      if ( (concurrent_child_fd = fork()) < 0) {
        write_log(new_log_fd, "fork error: failed to fork a child process to handle the client\n");
        return;
      } else if (concurrent_child_fd == 0) {
        close(sockfd);
        // write_log(new_log_fd, "Hello, TCP Daemon!\n");
        str_echo(accepted_sock_fd, new_log_fd);
        shutdown(accepted_sock_fd, SHUT_RDWR);
        exit(0);    /* For now, we won't do much, maybe later... */
      } else {
        close(accepted_sock_fd);
      }
    }
  }
}
