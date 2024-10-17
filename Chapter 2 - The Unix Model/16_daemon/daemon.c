/*
 * Usage:
 *  ->  Prepare the executable using the `make` command.
 *  ->  Run the executable using `./daemon`
 *  ->  NOTE: The following program does *nothing* as it is a skeleton for creating a daemon.
 *      It represents how a daemon is created. Some processes can be added to the skeleton 
 *      daemon to make it useful.
 *  ->  To remove the executable, run the `make clean` command.
*/

#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/param.h>

#include <unistd.h>     /* for getppid */
#include <sys/stat.h>   /* for umask */
#include <string.h>     /* for strlen */
#include <stdlib.h>     /* for exit */
#include <sys/ttycom.h> /* for the macro TIOCNOTTY */
#include <sys/ioctl.h>  /* for ioctl */
#include <sys/fcntl.h>  /* for the macro O_RDWR */

#include <errno.h>
extern int errno;

#ifdef SIGSTEP              /* true if BSD system */
  #include <sys/file.h>
  #include <sys/ioctl.h>
#endif

/* Function declarations to handle a daemon */
void sig_child        (int sig_id);
void daemon_start     (int ignore_sig_child);

/* Function declaration to handle error to standard error */
ssize_t err_sys       (const char *message); 
ssize_t write_string  (const char *message);

/* hard-coded entry point for a daemon */
int main (void) {
  
  printf("Initiating the daemon...\n");
  daemon_start(1);

  return 0;
}

/*
 * Write to standard error steam.
*/
ssize_t err_sys (const char *message) {
  size_t str_size = strlen(message);
  ssize_t characters_written;

  characters_written = write(2, message, str_size);

  return characters_written;
}

ssize_t write_string (const char *message) {
  size_t str_size = strlen(message);
  ssize_t characters_written;

  characters_written = write(1, message, str_size);

  return characters_written;
}

/* Handle the SIGCLD (in BSD) if the daemon process is interested in receiving the SIGCLD from the spawned processes. */
/* On System V, the SIG_IGN implicitly states the process is not interested in the exit status, so no zombie processes. */
void sig_child (int sig_id) {
  /* Can be used to check if BSD is defined. Check Appendix A.1 of the book for reference as well as Page 82. */
  
  /*
   * Use the wait3() system call with the WNOHANG option.
  */
  int pid;
  int status;
  /* union wait status; */

  /*    DEPRECATED: wait3 does not take a union of tag wait as the first argument
  while ((pid = wait3(&status, WNOHANG, (struct rusage *) 0)) > 0) {
    ;
  }
  */
  while ((pid = wait3(&status, WNOHANG, (struct rusage *) 0)) > 0) {
    ;
  }
}

/*
 * Usage: Detach a daemon process from login session context.
 * (param) ignore_sig_child: non-zero -> handle SIGCLDs so zombies don't clog.
*/
void daemon_start (ignore_sig_child)
int ignore_sig_child; {
  register int childpid, fd;

  /*
   * If we were started by init (process ID = 1) from the etc/inittab file
   * there's no need to detach.
   * This test is unreliable due to unavoidable ambiguity. (The parent process could call this function--the process has no wait function defined--terminates immediately such that the child process is an orphan, obtaining the parent process ID of 1.)
   * if the process is started by some other process and orphaned 
   * (i.e., if the parent process terminates before we are started.)
  */
  if (getppid() == 1) {
    goto out;
  }

  /*
   * Ignore the terminal stop signals (BSD)
  */
  #ifdef SIGTTOU
    signal(SIGTTOU, SIG_IGN);
  #endif  /* SIGTTOU */
  #ifdef SIGTTIN
    signal(SIGTTIN, SIG_IGN);
  #endif  /* SIGTTIN */
  #ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN);
  #endif  /* SIGTSTP */

  /*
   * If we were not started in the background, fork and let the parent exit.
   * This also guarantees the first child is not a process group leader.
  */
  if ( (childpid = fork()) < 0 ) {
    err_sys("can't fork first child.");
  } else if (childpid > 0) {
    exit(0);      /* parent */
  }

  /* 
   * First child process
   *
   * Disassociates from controlling terminal and process group. 
   * Ensure the process can't require a new controlling terminal.
  */
  #ifdef SIGTSTP    /* BSD */
    /* On my machine, setpgrp() takes no argument while setpgid takes the required (2) arguments. */
    if (setpgid(0, getpid()) == -1) {
      err_sys("can't change process group.");
    }
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

  /* NOTE: Daemon process will run here. */

  out:
    /*
     * Close any open file descriptors.
    */
    for (fd = 0; fd < NOFILE; fd++) {
      close(fd);
    }

    errno = 0;      /* probably got set to EBADF from a close. */

    /*
     * Move the current directory to root, to make sure we aren't on a mounted filesystem.
    */
    chdir("/");

    /*
     * Clear any inherited file mode creation mask.
    */
    umask(0);

    /*
     * See if the caller isn't interested in the exit status of its children and doesn't want to have them 
     * become zombies and clog up the system.
     * With System V, all we need to do is ignore the signal
     * With BSD, however, we have to catch each signal and execute the wait3 system call.
    */
    if (ignore_sig_child) {
      #ifdef SIGTSTP
        signal(SIGCHLD, sig_child);     /* Text Book has SIGCLD (for BSD), my machine has different macro name defined. */
      #else 
        signal(SIGCLD, SIG_IGN);        /* System V */
      #endif  /* SIGTSTP */
    }
}
