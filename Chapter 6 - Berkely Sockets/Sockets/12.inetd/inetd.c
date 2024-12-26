/*
 * Some of the steps which are done by `inetd` will be described below. We'll first discuss how it functions for stream socket 
 * (or TCP), and later discuss how it will function for datagram socket (UDP). The steps described are taken from the text.
 *
 * `inetd` looks into the `/etc/inetd.conf` to initialize itself. Some of the sample lines found in the file are:
 *
 *    ```
 *    service-name    socket-type     protocol      wait-flag       login-name      server-program      server-program-arguments
 *    ftp             stream          tcp           nowait          root            /etc/ftpd           ftpd
 *    telnet          stream          tcp           nowait          root            /etc/telnetd        telnetd
 *    login           stream          tcp           nowait          root            /etc/rlogind        rlogind
 *    tftp            datagram        udp           wait            nobody          /etc/tftpd          tftpd
 *    ```
 * NOTE: The first field provides the field-names. Refer to page 335 of the text.
 *
 *    Stream Socket.
 *    1.  On startup, it reads the `/etc/inetd.conf` file and creates a socket of appropriate type as specified in the file.
 *    2.  As each socket is created, a `bind` is executed for every socket, specifying the well-known address for the server.
 *        The port number is obtained by looking up the `service-name` field from the configuration file in the `/etc/services`
 *        file. Some sample lines from the `etc/services` are as follows:
 *
 *            ```
 *            ftp         21/tcp
 *            telnet      23/tcp
 *            tftp        69/udp
 *            login       513/tcp
 *            ```
 *
 *        Both the `service-name` and `protocol` from the inetd configuration file are passed as arguments to the library function
 *        `getservbyname` to locate the correct port number for the bind.
 *    3.  A `listen` is executed (for stream socket), speciftying a willingness to receive connections on the socket and the queue 
 *        length for incoming connections.
 *    4.  A `select` is then executed, to wait for the first socket to become ready for reading. A stream socket is considered 
 *        "ready for reading" when a connection request arrives for that socket. A datagram socket is ready for reading when a 
 *        datagram arrives.
 *        At this point, the `inetd` daemon just waits for the select system call to return.
 *    5.  When a socket is ready for reading, an accept system call is executed (for stream socket) to accept the connection.
 *    6.  The `inetd` daemon `fork`s and the child process handles the service request. The child closes all the file descriptors 
 *        other than the socket descriptor that it is handling and then calls `dup2` to cause the socket to be duplicated on 
 *        file descriptors 0, 1, and 2. The original socket descriptor is then closed. Doing this, the only file descriptors that 
 *        are open in the child are 0, 1, and 2. 
 *        It then calls the `getpwnam` to get the password file entry for the `login-name` that is specified in the `/etc/inetd.conf`
 *        file. If this entry does not have a user ID of zero (the superuser) then the child becomes the specified user by executing 
 *        the `setgid` and `setuid` system calls. (Since the `inetd` process is executing with a userID of zero, the child process 
 *        inherits this user ID across the `fork`, so it is able to become any user it chooses.) The child process now does an `exec`
 *        to execute the appropriate `server-program` to handle the request, passing the arguments that are specified in the 
 *        configuration file.
 *    7.  The parent process must close the connected socket (for the stream socket). The parent goes back and executes the `select` 
 *        system call again, waiting for the next socket to become ready for reading.
 *
 *    NOTE: The scenario described above is for `nowait` for the server. If another connection request arrives for the same server, 
 *          it is returned to the parent process as soon as it executes the `select`. The steps listed above are executed again, and 
 *          another child process handles the new service request. (Granted, we have specified the `backlog` argument for the 
 *          respective stream socket to be > 1. Then only can the server provide service to more than one connection request.)
 *
 *
 *    Datagram socket.
 *    1.  The datagram request arrives on Socket `N`, the `select` returns to the `inetd` process.
 *    2.  A child process is `fork`ed and `exec`ed to handle the request.
 *    3.  `inetd` disables socket descriptor `N` from its fd_set structure for the `select`. The child process takes over 
 *        the socket.
 *    4.  The child handles this request and `inetd` handles requests for other services.
 *    5.  Eventually, `inetd` calls `select` and blocks.
 *    6.  The child terminates, the `SIGCLD` (now `SIGCHLD`) signal is generated for the `inetd` process.
 *    7.  `inetd` handles the signal and obtains the process ID of the terminating child process from the `wait` system call.
 *        It figures out which socket descriptor corresponded to this child process and turns on the appropriate bit in its 
 *        `fd_set` structure. 
 *    8.  When the signal handler returns, the `select` returns to the `inetd` process, with an `errno` of `EINTR`.
 *    9.  `inetd` calls `select` again, this time with an `fd_set` structure that enables socket descriptor `N` .
 *  
 * The code below will obviously have some changes made. I will try to conform to the steps, but with some minor changes for making 
 * it more easier to read. And, no, I won't be using `exec` for now, as it in-turn calls another service-specific daemon. This 
 * program will only illustrate how `inetd` daemon works.
*/

/*
 * A bit of rant about `exec` family function as it's used here, and may even be modified if it seems appropriate.
 * There are 6 `exec` functions:
 *  1.  execlp  (file, arg, ..., 0);
 *  2.  execvp  (file, argv);
 *  3.  execl   (path, arg, ..., 0);
 *  4.  exevcv  (path, argv);
 *  5.  execle  (path, arg, ..., 0, envp);
 *  6.  execve  (path, argv, envp);
 *
 * -> Notice the first two function takes the `file` argument, which specifies the name of the executable. When one of the two calls is used, 
 *    the path to the executable is determined from the PATH environment variable. If PATH is not defined in current environment, then the 
 *    default one (":/bin:/usr/bin") is used. If `execlp` or `execvp` `pathname` (`file` used above) argument contains the `/` anywhere in the 
 *    argument, then the PATH is not used. Rest of the function requires fully qualified `pathname`.
 * -> The calls in 1, 3, and 5 takes individual strings (char *) as arguments (with NULL, i.e., (char *) 0) to indicate final argument, 
 *    while 2, 4, and 6 takes an array of strings (char ** or (char *([])) ) which includes the NULL string as the final argument.  
 * -> Calls except for 5 and 6 do not take the pointer to environmnet variable, so the current value of external variable `environ` is used
 *    for building an environment list that is passed to the new program. The other two defines an explicit variable (an array of string)
 *    which is an array of pointers must be terminated by a NULL pointer.
 *
*/

#include "utils.h"

#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>


#define   PATH_LEN  512

extern int errno;

/*
 * Surely, there's a better way to handle distinct services. But this is a naive approach due to my 
 * lack of knowledge. (Only read till Chapter 6 :( ...)
*/
typedef struct services {
  int                   sock_type;          /* 0 for stream, 1 for datagram */
  int                   port_number;        /* 32-bit protocol number, must use htons() */
  char                  service[16];        /* name of the service; str_echo, dg_echo, str_dis, dg_dis */
  int                   sockfd;             /* socket descriptor returned from `socket` call */
  struct sockaddr_in    serv_addr;          /* network address of the server, used during `bind` */
  /* NOTE: NOT USED BY UDP SERVICES */
  struct sockaddr_in    cli_addr;           /* client's network address, used during `accept` */
  int                   cli_addr_len;       /* client's network address length, initialize it, but is a value-result arg for `accept` */
  /* NOTE: NOT USED BY TCP SERVICES */
  pid_t                 child_pid;          /* process ID of the child process, used by datagram sockets */
} services;

int         udp_select_conn     = 0;        /* `select`ed for reading datagram sockets, need to make sure they aren't again set. */
fd_set      read_sockfds;                   /* set of file descriptors, used by `select` */
services    serv_arr[4];                    /* replacement for `inetd`'s way of reading from file, naive approach */

void sig_child        (int sig_id);
void daemon_start     (int ignore_sig_child);

/*
 * There are some issues, mostly regarding the `str_dis` service. 
 * In normal scenario, the protocol services runs indefinitely, 
 * but for testing purpose, I've added the "termination" command.
 * When the client sends the terminating command for `str_dis` service
 * at the start, it works as intended, but if it's later sent, then the server
 * won't receive it. My assumption is that the client program (in my machine)
 * lacks the mechanism to handle scenario when the server sends nothing. So, it will
 * remain the "blocking" mode, although the client can use the standard input stream 
 * to read more data. To check if it's working as expected, try to put send the message 
 * back to the client and later occurrance of "terminating" command will work.
 *
 * UDP services work as expected too, and contain the "terminating" command. 
 * When the terminating command is received by the server, it closes the service and exits, 
 * making the service again "ready-to-read". The client won't know about this because of the 
 * connection-less property, so if it again sends a messsage to the client, the service is 
 * re-run. One more thing, the datagram service keeps track of the first client's port address,
 * so if other client attempts to write to the server, the server will neglect it.
*/
int main (void) {

  int                 logfd;                                            /* log file descriptor, as daemon can't use the terminal. */
  int                 accept_sockfd;                                    /* used by stream sockets */
  int                 pid;                                              /* used when `fork`ing */             
  int                 sockaddr_in_len = sizeof(struct sockaddr_in);     /* size of Internet address structure, 16 bytes*/
  int                 max_sockfd      = 0;                              /* used to keep track of highest file descriptor, used by `select` */
  int                 clear_fd        = 0;                              /* used by datagram socket before service. */

  /* basic chores */
  FD_ZERO(&read_sockfds);

  /* For str_echo */
  serv_arr[0].sock_type     = 0;
  serv_arr[0].port_number   = 6969;
  serv_arr[0].cli_addr_len  = sizeof(struct sockaddr_in);
  serv_arr[0].child_pid     = -1;   /* TCP won't need this */
  strlcpy(serv_arr[0].service, "str_echo", sizeof(serv_arr[0].service));

  /* For dg_echo */
  serv_arr[1].sock_type     = 1;
  serv_arr[1].port_number   = 6969;
  serv_arr[1].cli_addr_len  = sizeof(struct sockaddr_in);
  serv_arr[1].child_pid     = 0;    /* initialize for UDP */
  strlcpy(serv_arr[1].service, "dg_echo", sizeof(serv_arr[1].service));

  /* For stream discard service */
  serv_arr[2].sock_type     = 0;
  serv_arr[2].port_number   = 6970;
  serv_arr[2].cli_addr_len  = sizeof(struct sockaddr_in);
  serv_arr[2].child_pid     = -1;
  strlcpy(serv_arr[2].service, "str_dis", sizeof(serv_arr[2].service));

  /* For datagram discard service */
  serv_arr[3].sock_type     = 1;
  serv_arr[3].port_number   = 6970;
  serv_arr[3].cli_addr_len  = sizeof(struct sockaddr_in);
  serv_arr[3].child_pid     = 0;
  strlcpy(serv_arr[3].service, "dg_dis", sizeof(serv_arr[3].service));

  /* get current working directory, where the executables reside */
  char cwd[PATH_LEN + 1];
  getcwd(cwd, PATH_LEN);
  int path_len = strlen(cwd);
  cwd[path_len] = '/';          /* could add this to member `service` above, but this is better. */
  cwd[path_len + 1] = '\0';     /* extra secure :) */

  daemon_start(1);
  /* 
   * Set the current process group ID as the process ID of this process ID. (the new process group leader)
   * Removed the association with the controlling terminal.
   * Removed all the file descriptors (except fd 0, 1, and 2. They will be redirected once select returns a connection.)
   * Changed the current directory for process to "/"
   * Changed the umask to 0. (was probably 0022 before.)
   * Activated the signal handler for SIGCHLD.
   * `errno` set to 0.
  */

  const char *log_path = "./tmp/inetd.txt";
  char        exec_path[PATH_LEN + 1];

  /* 
   * Open the log file, localed in `/tmp/` directory. If the file doesn't exist, create file, and 
   * add the file mode will be `0-666` (read-write for user, group, and others). Then open in 
   * read/write mode. If the file already exists, open it in append mode, i.e. lseek the file 
   * at the end-of-file, and start appending the data from there. 
  */
  if ( (logfd = open(log_path, O_RDWR | O_APPEND | O_CREAT, 0666)) < 0 ) {
    exit(EXIT_FAILURE);
  }

  /*
   * Initialize all the sockets, bind them as well. `inetd` reads the file to do this, but I created a basic structure for this.
  */
  for (int i = 0; i < (int) ARR_ELE_CNT(serv_arr); i++) {
    switch (serv_arr[i].sock_type) {
      case 0:   /* stream socket */
        if ( (serv_arr[i].sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
          write_log(logfd, "socket error: failed to create a stream socket\n");       
          exit(EXIT_FAILURE);
        }
        bzero(&(serv_arr[i].serv_addr), sizeof(struct sockaddr_in));
        serv_arr[i].serv_addr.sin_family      = AF_INET;
        serv_arr[i].serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_arr[i].serv_addr.sin_port        = htons(serv_arr[i].port_number);
        if ( bind(serv_arr[i].sockfd, (struct sockaddr *) &(serv_arr[i].serv_addr), sockaddr_in_len) < 0) {
          write_log(logfd, "bind error: failed to bind well known address for stream socket\n");
          exit(EXIT_FAILURE);
        }
        /* Stream socket specific */
        if (listen(serv_arr[i].sockfd, 5) < 0) {
          write_log(logfd, "listen error: failed to listen to stream socket binded to well known address\n");
        }
        if (serv_arr[i].sockfd > max_sockfd) {
          max_sockfd = serv_arr[i].sockfd;
        }
        /* set the socket for `select` */
        FD_SET(serv_arr[i].sockfd, &read_sockfds);
        break;
      case 1:
        if ( (serv_arr[i].sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
          write_log(logfd, "socket error: failed to create a datagram socket\n");
          exit(EXIT_FAILURE);
        }
        bzero(&(serv_arr[i].serv_addr), sizeof(struct sockaddr_in));
        serv_arr[i].serv_addr.sin_family      = AF_INET;
        serv_arr[i].serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_arr[i].serv_addr.sin_port        = htons(serv_arr[i].port_number);
        if ( bind(serv_arr[i].sockfd, (struct sockaddr *) &(serv_arr[i].serv_addr), sockaddr_in_len) < 0) {
          write_log(logfd, "bind error: failed to bind well known address for stream socket\n");
          exit(EXIT_FAILURE);
        }
        if (serv_arr[i].sockfd > max_sockfd) {
          max_sockfd = serv_arr[i].sockfd;
        }
        /* set the socket for `select` */
        FD_SET(serv_arr[i].sockfd, &read_sockfds);
        break;
      default:
        exit(EXIT_FAILURE);
    }
  }

  for (;;) {
    /* 
     * Wait for any of the socket to become ready for reading, i.e., stream socket waits till a connection
     * is requested by the client, and datagram socket waits till the client sends a message to the socket.
    */
    if (select(max_sockfd + 1, &read_sockfds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0) < 0) {
      /* May be interrupted when a child process terminates. EINTR is returned in such case. */
      if (errno == EINTR) {
        continue;
      }
      /* errno apart from EINTR. terminate. */
      write_log(logfd, "select error: failed to select a socket available for reading\n");
      exit(EXIT_FAILURE);
    }
    /* 
     * cwd contains the current working directory, exec_path contains the path to the executable.
     * The reason to have two of them is cause the program will later use `strlcat` to concatenate 
     * the executable name to the path, and the program needs to handle multiple requests.
    */
    bzero(exec_path, sizeof(exec_path));
    strlcpy(exec_path, cwd, sizeof(exec_path));
    for (int i = 0; i < (int) ARR_ELE_CNT(serv_arr); i++) {
      if (FD_ISSET(serv_arr[i].sockfd, &read_sockfds)) {
        /* reached here if one of the socket is ready for reading. */
        /*
         * Some common things done by all of the connection:
         *  
         *  TCP:
         *    1.  "zero" out the structure member `cli_addr` before sending it to `accept` (value-result).
         *    2.  concatenate the executable file to the path.
         *    3.  `fork` the process, the child process closes all the descriptor execpt for `accept`ed one,
         *        use `dup2` to redirect the fd 0, 1, and 2 to `accept`ed socket, and close the `accept`ed socket.
         *    4.  In the child process, use `execl` function to execute the executable.
         *    5.  In the parent process, close the `accept`ed socket, and break from the loop. This loop is used to traverse through 
         *        each socket descriptor and check if it's ready for reading. If one is found, we can break.
         *
         *  UDP:
         *    1.  Copy the socket descriptor which is ready for reading into `clear_fd`
         *    2.  Concatenate the executable to the path.
         *    3.  `fork` the process, and in the child process, close all the descriptor execpt for `ready-to-read` socket descriptor.
         *        Then `dup2` the socket descriptor to fd 0, 1, and 2, and then close it as well. Call the `execl` function to execute 
         *        the server.
         *    4.  In the parent process, keep track of the process ID of child process (in member `child_pid`). This is done to, a) clear
         *        out the file descriptor as the datagram socket is not connection-oriented, and may only have one client, and b) use it 
         *        to check if any recent child process which terminated, resembles the stored `child_pid`, to again set the socket for 
         *        `ready-to-read`. `udp_select_conn` is used for similar purpose.
         *    5.  break out of the loop.
         *
         * One final note: the second argument to `execl` function, which is the first argument to the executable, need not have any speicific 
         * format, i.e. `./<prog-name>` or `<prog-name>`. It can be anything (afaik even random names). For simplicity, I chose the `<prog-name>`
        */
        if (serv_arr[i].sock_type == 0) {                                         /* stream socket */
          if(strcmp(serv_arr[i].service, "str_echo") == 0) {                      /* str_echo */
            bzero(&(serv_arr[i].cli_addr), sizeof(struct sockaddr_in));
            if ( (accept_sockfd = accept(serv_arr[i].sockfd, (struct sockaddr *) &(serv_arr[i].cli_addr), (socklen_t *) &(serv_arr[i].cli_addr_len))) < 0) {
              write_log(logfd, "accept error: failed to accept the connection request\n");
              exit(EXIT_FAILURE);
            }
            strlcat(exec_path, serv_arr[i].service, sizeof(exec_path));
            if ( (pid = fork()) < 0) {
              write_log(logfd, "fork error: failed to fork the process\n"); 
              exit(EXIT_FAILURE);
            } else if (pid == 0) {              /* child process */
              close(serv_arr[i].sockfd);
              for (int i = 0; i < NOFILE; i++) {
                if (accept_sockfd == i) {
                  continue;
                }
                close(i);
              }
              dup2(accept_sockfd, 0);
              dup2(accept_sockfd, 1);
              dup2(accept_sockfd, 2);
              close(accept_sockfd);

              /*
               * NOTE:
               *  Rather than using the `execl` function, one can also use the simplified `execlp` function. This is because the 
               *  function `execlp` function takes the `path` argument (a C-string) and if the string did not contain any forward 
               *  slash (/), then the file would be searched using the PATH environment variable. On my system's manual page, the 
               *  section for `execlp` function (along with `execvp` and `execvP` function) states that:
               *  
               *      ```
               *      The functions execlp(), execvp(), and execvP() will duplicate the actions of the shell in searching for an 
               *      executable file if the specified file name does not contain a slash “/” character.  For execlp() and execvp(), 
               *      search path is the path specified in the environment by “PATH” variable.  If this variable is not specified, 
               *      the default path is set according to the _PATH_DEFPATH definition in <paths.h>, which is set to “/usr/bin:/bin”.  
               *      For execvP(), the search path is specified as an argument to the function.  In addition, certain errors are treated specially.
               *      ```
               *  
               *  Under compatibility, the manual page also states that:
               *
               *      ```
               *      Historically, the default path for the execlp() and execvp() functions was “:/bin:/usr/bin”.  This was changed 
               *      to place the current directory last to enhance system security.
               *      ```
               *
               *  So, replacing the call to `execlp` with the first argument as `serv_arr[i].service` will have identicial effect.
              */
              if (execl(exec_path, serv_arr[i].service, (char *) 0) < 0) {
                write_log(logfd, "execl error: failed to execute the required program\n");
                exit(EXIT_FAILURE);
              }
            } else {                            /* parent process */
              close(accept_sockfd);     /* close the `accept`ed socket descriptor as the parent need not use it. */
              break;
            }
          } else if (strcmp(serv_arr[i].service, "str_dis") == 0) {                 /* str_dis */
            bzero(&(serv_arr[i].cli_addr), sizeof(struct sockaddr_in));
            if ( (accept_sockfd = accept(serv_arr[i].sockfd, (struct sockaddr *) &(serv_arr[i].cli_addr), (socklen_t *) &(serv_arr[i].cli_addr_len))) < 0) {
              write_log(logfd, "accept error: failed to accept the connection request\n");
              exit(EXIT_FAILURE);
            }
            strlcat(exec_path, serv_arr[i].service, sizeof(exec_path));
            if ( (pid = fork()) < 0) {
              write_log(logfd, "fork error: failed to fork the process\n"); 
              exit(EXIT_FAILURE);
            } else if (pid == 0) {              /* child process */
              close(serv_arr[i].sockfd);
              for (int i = 0; i < NOFILE; i++) {
                if (accept_sockfd == i) {
                  continue;
                }
                close(i);     /* close all descriptor except one which was returned from `accept` */
              }
              dup2(accept_sockfd, 0);
              dup2(accept_sockfd, 1);
              dup2(accept_sockfd, 2);
              close(accept_sockfd);

              if (execl(exec_path, serv_arr[i].service, (char *) 0) < 0) {
                write_log(logfd, "execl error: failed to execute the required program\n");
                exit(EXIT_FAILURE);
              }
            } else {                            /* parent process */
              close(accept_sockfd);     /* close the `accept`ed socket descriptor as the parent need not use it. */
              break;
            }
          }
        } else {                              /* datagram socket */
          clear_fd = serv_arr[i].sockfd;
          /* fork and execute the function */
          if (strcmp(serv_arr[i].service, "dg_echo") == 0) {                        /* dg_echo */
            strlcat(exec_path, serv_arr[i].service, sizeof(exec_path));
            if ( (pid = fork()) < 0) {
              write_log(logfd, "fork error: failed to fork the process\n"); 
              exit(EXIT_FAILURE);
            } else if ( pid == 0 ) {            /* child process */
              /* clear out the other file descriptor */
              for (int i = 0; i < NOFILE; i++) {
                if (clear_fd == i) {
                  continue;
                }
                close(i);
              }
              dup2(serv_arr[i].sockfd, 0);
              dup2(serv_arr[i].sockfd, 1);
              dup2(serv_arr[i].sockfd, 2);
              close(serv_arr[i].sockfd);

              if (execl(exec_path, serv_arr[i].service, (char *) 0) < 0) {
                write_log(logfd, "execl error: failed to execute the required program\n");
                exit(EXIT_FAILURE);
              }
            } else {                            /* parent process */
              serv_arr[i].child_pid = pid;
              udp_select_conn++;
              break;
            }
          } else if (strcmp(serv_arr[i].service, "dg_dis") == 0) {                      /* dg_dis */
            strlcat(exec_path, serv_arr[i].service, sizeof(exec_path));
            if ( (pid = fork()) < 0) {
              write_log(logfd, "fork error: failed to fork the process\n"); 
              exit(EXIT_FAILURE);
            } else if ( pid == 0 ) {            /* child process */
              /* clear out the other file descriptor */
              for (int i = 0; i < NOFILE; i++) {
                if (clear_fd == i) {
                  continue;
                }
                close(i);
              }
              dup2(serv_arr[i].sockfd, 0);
              dup2(serv_arr[i].sockfd, 1);
              dup2(serv_arr[i].sockfd, 2);
              close(serv_arr[i].sockfd);

              if (execl(exec_path, serv_arr[i].service, (char *) 0) < 0) {
                write_log(logfd, "execl error: failed to execute the required program\n");
                exit(EXIT_FAILURE);
              }
            } else {                            /* parent process */
              serv_arr[i].child_pid = pid;
              udp_select_conn++;
              break;
            }
          }
        }
      }
    }
    /* NOTE: REACHES HERE AFTER GETTING THE `READY-TO-READ` SOCKET, AND `EXEC`S THE APPROPRIATE EXECUTABLE */
    /* 
     * Obviously, there's a better way to handle this, but it is just a "will-do" solution.
     * First, all the socket descriptor are again set. Then, if `udp_select_conn` is set (non-zero), 
     * the program recently `exec`ed one of the datagram (UDP) service. Then, we again check which 
     * service was used, and clear out the appropriate socket from reading by `select`.
    */
    for (int j = 0; j < (int) ARR_ELE_CNT(serv_arr); j++) {
      FD_SET(serv_arr[j].sockfd, &read_sockfds);
    }
    if (udp_select_conn) {
      for (int k = 0; k < (int) ARR_ELE_CNT(serv_arr); k++) {
        if (serv_arr[k].sock_type == 1 && serv_arr[k].child_pid != 0) {
          FD_CLR(serv_arr[k].sockfd, &read_sockfds);
        }
      }
    }
  }

  /* NOT REACHABLE. */
  
  /* close all the descriptors and exit */
  for (int i = 0; i < (int) ARR_ELE_CNT(serv_arr); i++) {
    close(serv_arr[i].sockfd);
  }
  exit(EXIT_SUCCESS);
}

void sig_child (int sig_id) {
  int pid;
  int status;

  if (sig_id != SIGCHLD) {
    return;
  }

  /* 
   * Naive approach as we aren't really checking the status of the terminated/signalled child process. 
   * The `-1` argument indicates the we aren't looking for a specific child process, but rather any.
   * WNOHANG indicates that the `waitpid` be non-blocking call.
  */
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    for (int i = 0; i < (int) ARR_ELE_CNT(serv_arr); i++) {
      if ( pid == serv_arr[i].child_pid ) {
        FD_SET(serv_arr[i].sockfd, &read_sockfds);
        udp_select_conn--;
        serv_arr[i].child_pid = 0;
        break;
      }
    }
    break;
  }
}

void daemon_start (int ignore_sig_child) {
  register int childpid, fd;

  /* 
   * If the `initd` (or `launchd` in my machine) called this function, no need to do extra `fork`ing,
   * just close the descriptors, unmount to root ("/") directory, change process's file mask (typically 0-022, octal yes)
   * check if the calling process wants to handle SIGCHLD signals (to avoid zombie process) (optional), and open the 
   * "/dev/null" and assure it's occupying file descriptor 0, then `dup2` it to file descriptor 1, and 2 (only required for 
   * this case).
  */
  if (getppid() == 1) {
    goto out;
  }

  signal(SIGTTOU, SIG_IGN);     /* Ignore the signal generated if the process (in the background) attempts to write to control terminal */
  signal(SIGTTIN, SIG_IGN);     /* Ignore the signal generated if the process (in the background) attempts to read from control terminal */
  signal(SIGTSTP, SIG_IGN);     /* Ignore the signal genrated if process receives suspend key (CTRL-Z) or delayed suspend key (CTRL-Y) */

  /* First fork, and exit the parent out, the child is orphaned (ppid = 1) */
  if ( (childpid = fork()) < 0 ) {
    perror("can't fork first child.");
    return;
  } else if (childpid > 0) {
    exit(0);      /* parent */
  }

  /* First child process */

  #ifdef SIGTSTP    /* BSD */
    /* Set process group ID as the process ID of the process, making it the process group leader or session leader. */
    if (setpgid(0, getpid()) == -1) {
      perror("can't change process group.");
      return;
    }
    /* 
     * Open this "special" file, and use TIOCNOTTY flag to tell the system that this process wishes to detach from the 
     * controlling terminal. Hence, we can't access the terminal which invoked this process. 
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

    /* Ensure that descriptor 0, 1, and 2 are not taken by sockets. */
    fd = open("/dev/null", O_RDWR);
    if (fd == 0) {
      dup2(fd, 1);
      dup2(fd, 2);
    } else {
      exit(EXIT_FAILURE);
    }
}
