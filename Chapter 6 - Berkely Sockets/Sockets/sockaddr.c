#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_endian.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/errno.h>
#include <string.h>

/*
 *
 * socket function declaration - int socket(int family, int type, int protocol);
 *
 * Some valid combinations:
 *
 * |    type      |AF_UNIX|AF_INET|AF_NS|
 * |--------------|-------|-------|-----|
 * |SOCK_STREAM   | Yes   | TCP   | SPP |
 * |SOCK_DGRAM    | Yes   | UDP   | IDP |
 * |SOCK_RAW      |  -    | IP    | Yes |
 * |SOCK_SEQPACKET|  -    | -     | SPP |
 *
 * NOTE: The AF prefix on the `int family` param stands for Address Family, which is similar to the PF prefix, called the Protocol Family.
 * NOTE: The boxes marked "Yes" are valid, but don't have handy acronyms. The empty boxes (indicated by `-`) are not implemented.
 *
 * protocol is usually 0, but there are specialized application that uses specific protocol. A few of them are listed below.
 *
 * |  family  |     type        |   protocol    | Actual protocol |
 * |----------|-----------------|---------------|-----------------|
 * |  AF_INET | SOCK_DGRAM      | IPPROTO_UDP   | UDP             |
 * |  AF_INET | SOCK_STREAM     | IPPROTO_TCP   | TCP             |
 * |  AF_INET | SOCK_RAW        | IPPROTO_ICMP  | ICMP            |
 * |  AF_INET | SOCK_RAW        | IPPROTO_RAW   | (raw)           |
 * |  AF_NS   | SOCK_STREAM     | NSPROTO_SPP   | SPP             |
 * |  AF_NS   | SOCK_SEQPACKET  | NSPROTO_SPP   | SPP             |
 * |  AF_NS   | SOCK_RAW        | NSPROTO_ERROR | Error Protocol  |
 * |  AF_NS   | SOCK_RAW        | NSPROTO_RAW   | (raw)           |
 *
 * NOTE: My system does not provide the macros of kind NSPROTO_, probably cause the system does not support Xerox Networking System's Interfaces. (as well as no header directory nor the header file <netns/ns.h>)
*/

/*
 * NOTE: We have discussed the 5-tuple association in the previous chapter, which has the following format:
 *    
 *    { protocol, local address, local process (port), foreign address, foreign process (port) }
 * 
 * As mentioned by the text, the `socket()` system call is only responsible for assigning the protocol.
 * For a socket descriptor to be of any use, the information regarding the other four (4) members of the tuple should be provided.
 *
 * The table below shows the various system calls in regards to the 5-tuple association defined above:
 *
 *    |     type of service           |   protocol  |   local-addr, local-process   |   foreign-addr, foreign-process   |
 *    |-------------------------------|-------------|-------------------------------|-----------------------------------|
 *    | connection-oriented (server)  |   socket()  |           bind()              |           listen()                |
 *    | connection-oriented (client)  |   socket()  |                           connect()                               |
 *    | connectionless (server)       |   socket()  |           bind()              |           recvfrom()              |
 *    | connectionless (client)       |   socket()  |           bind()              |           sendto()                |
 *
 * It is highly recommended that the reader refers to the manual provided by the system (man (1)) for the above mentioned system calls.
*/

extern int errno;

/*
 * So, what's happening here?
 * What we're doing is first describing the socket (an end-point where other socket is connected through the internetwork (like the  
 * Internet) which contains the necessary information about the connection 5-tuple association (not all) (such as foreign address, and 
 * foreign process (port)). Be aware that the `network_address` variable we described to "connect" to the server with our defined 
 * socket (`sockfd`) contains information about the server address and it's server process. `struct sockaddr_in` is a specific 
 * structure defined in <netin/in.h>. The system defines a generic socket address structure, `sockaddr`, which contains 16-byte 
 * data (socket address family and socket address data--the network address, like 4 bytes for Internet and 10 bytes for XNS) where 
 * the family might be used for describing the type of network the socket acts as with the networking interface (like INET (Internet),
 * NS (XNS), SNA (SNA/LU), etc) and the address is the actual network address for routing through the internetwork. 
 *
 * The running process (or socket defined in that process) can "bind" with the socket address (struct sockaddr) to denote that the 
 * process wants to receive any packets that are sent to the respective address defined. Notice that `bind` takes the socket address 
 * **as well as** the socket descriptor (an interface to the transport layer provided by kernel) so that the socket essentially is 
 * a "named pipe"--where the name is the network address to an pipe for IPC--being tracked by the kernel until the socket descriptor 
 * is closed.
 *
 * A process can also "connect" the socket descriptor with another socket whose address and process are defined. Recall that `connect` 
 * system call takes the socket descriptor **as well as** the structure of socket address (sockaddr) which contains the information about 
 * foreign process (and address) that the client wants to connect to. We also need to pass the size of the structure for the second 
 * argument, because the size of network address for one network can be different from the other (Internet and XNS, for instance). This 
 * is the "generic solution" to this problem, although the author of the text proposes alternate idea using `union`. For this, we use the
 * Internet specific network stack's structure (sockaddr_in) which is of size 16 bytes as well. 
 *
 * As the socket system calls require the sockaddr structure (also 16 bytes) as arguments, we need to cast the specific network structure
 * into the generic structure which may be later handled by the kernel. The reason I'm stressing on these points is because sockets are 
 * not limited to network communication, but can be used for local process communication as well. The structure of socket address used 
 * for local process communication is different (108 bytes plus the socket address family), and it the task of the system to handle the 
 * process communication through sockets (I think somewhat similar to other IPCs like message queue, shared memory). For now, we will
 * just focus on Internet connection through sockets and I will cover local communication later...
 *
 * For now, I will use the Internet address assigned as the "loopback" address (127.0.0.1) which, when the system detects, refers to the 
 * host itself rather than sending the packet to the internetwork via network interface. We define the port where the process in "remote"
 * system is listening to (6969). We specify that the socket is of address family INET (Internet).
 *
 * If _CHECK_TCP flag is defined, the socket we defined is a connection-oriented one rather than the default connectionless one. As the 
 * TCP connection attempts to connect to the foreign address and process, there should also exist the foreign process (6969) listening to 
 * that address (127.0.0.1). This is done in `bind.c` file which also requires the _CHECK_TCP flag to be enabled. If we do not initialize
 * the server first, the connection is not established, hence an error occurs.
 *
 * The normal program will not have any issues as the connection made is connectionless and uses the UDP transport layer protocol. 
 *
 * One last note: We can replace the loopback address with the network address provided by the router (use the ifconfig and look 
 * for en0 interface which contains the address) and the program will still work as intended (using the _CHECK_TCP flag). The 
 * question now is how to make the program act as a server binded to the public Internet address the router uses to access the 
 * Internet, which can be found from sites on the web, if not TCP, even UDP one for that.
*/
int main (void) {
  
  // struct sockaddr in_socket;

  struct sockaddr_in in_details;            // Internet socket address structure

  // struct sockaddr_ns xerox_details;      // my system does not have the <netns/ns.h> header
  
  // struct sockaddr_un unix_details;          // Unix socket address structure
  
  unsigned short port = (unsigned short) 6969;

  struct in_addr network_address;
  inet_aton("127.0.0.1", &network_address);

  in_details.sin_family   = AF_INET;
  in_details.sin_addr     = network_address;
  /* htons (host to network short): convert the 16-bit integer short to network byte order equiavelent */
  in_details.sin_port     = htons(port);
  /* 6969 (base 10) = 0x1B39 (base 16). Notice that the result is 0x391B (big endian (network byte order) representation)*/
  printf("[LOG] The unsigned short representation of %hu (in hex) is: 0x%X\n", port, htons(port));

  /* MISC:  We can create a socket with the protocol as tcp. 
   *        Be aware that tcp is connection-oriented, so when we try to `connect` the socket we created with the server
   *        (which is localhost in our case), then there must be a process listening to port (6969) in this case.
   *        If this condition is not met, the `connect` system call fails with errno set.
   *
   *        One simple hack is:
   *          1. Goto the repo here: https://github.com/PranavRJoshi/Sockets-in-C.git
   *          2. Clone the repo to your local system, and then navigate to the repo.
   *          3. Navigate to HTTP Socket directory.
   *          4. Prepare the executable using the `make` command.
   *          5. run the executable `./http_server`
   *
   *        After all of this is done, you can uncomment the code below. (make sure the in_details.sin_port = htons((unsigned short) 8001)
   *        and then try to connect.)
   *
   * NOTE:  An easier way, if you don't want to go around and do the steps above. 
   *        In the current directory, check and make sure the Makefile contains the `test` target.
   *        run the `make test` command, which creates two executables: ./server (from bind.c) and ./client (from sockaddr.c)
   *        Notice that it enables the _CHECK_TCP flag.
  */
  /*
   *  int tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
   *  unsigned short tcp_port = (unsigned short) 8001;
   *  in_details.sin_port = htons(tcp_port);
   *  
   *    if (connect(tcp_sockfd, (struct sockaddr *) &in_details, sizeof(in_details)) < 0) {
   *      perror("[ERROR] Failed to establish a connection-oriented service with the remote address specified");
   *      exit(EXIT_FAILURE);
   *    } else {
   *      fprintf(stdout, "[SUCCESS] Connection-oriented service established with localhost.");
   *    }
   *
   *    close(tcp_sockfd);
  */


  #ifndef _CHECK_TCP    /* Default behavior of UDP */

  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  /* the protocol argument can be set as 0 (instead of IPPROTO_UDP (explicit)) */

  /* 
   * `connect` will not fail as the service we're using--UDP--is a connectionless service. 
   * The reason it returns immediately, rather than establishing a connection-oriented service 
   * is because this program acts as a "connectionless client", which tries to "connect" to the 
   * localhost. If we have a localhost with port 6969 running, and then any the server sends to 
   * this process (datagram, not stream/messages), then it is received by this socket descriptor.
  */
  if (connect(sockfd, (struct sockaddr *) &in_details, sizeof(in_details)) < 0) {
    fprintf(stderr, "[ERROR] errno: %d\n", errno);
    perror("[ERROR] Failed to connect the socket to localhost.\n");
  } else { 
    printf("[SUCCESS] Socket connected to localhost with connection-less service. Closing...\n");
    const char *message = "Hello, UDP!";
    write(sockfd, message, strlen(message));
  }
  #else /* !TCP */
  /* Testing purpose only... */
  /* run the `make test` command and start the server (which is ./server) and then only run `./client` */
  int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (connect(sockfd, (struct sockaddr *) &in_details, sizeof(in_details)) < 0) {
    fprintf(stderr, "[ERROR] errno: %d\n", errno);
    perror("[ERROR] Failed to connect the socket to localhost.\n");
  } else { 
    printf("[SUCCESS] Socket connected to localhost with connection-oriented service. Closing...\n");
    char message[256] = "Hello, TCP!";
    write(sockfd, message, sizeof(message));
  }
  #endif

  close(sockfd);

  //  SOCKET PAIR
  /*
   * This is another one of the system calls provided by Unix-like systems to allow Unix Domain sockets... in a pair.
   * The function prototype for the system call is: 
   *
   *    int socketpair (int family, int type, int protocol, int sockvec[2]);
   *
   * Since it works for Unix Domain only, there are only two allowable versions of it.
   *
   *    1.  socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd);    // sockfd is a 2-element array of integers.
   *    
   *    2.  socketpair(AF_UNIX, SOCK_DGRAM, 0, sockfd);
   *
   * Unlike pipes, which are uni-directional, socket pairs are bidirectional, connection service is described by the `type` argument, 
   * and Unix domain socket stream pipes.
  */

  int sockpairfd[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockpairfd) < 0) {
    fprintf(stderr, "[ERROR] errno = %d\n", errno);
    perror("[ERROR] Failed to create socket pair. Exiting.\n");
    exit(EXIT_FAILURE);
  } else {
    fprintf(stdout, "[SUCCESS] Created two socket descriptor. Closing them...\n");
  }

  close(sockpairfd[0]);
  close(sockpairfd[1]);

  return 0;
}
