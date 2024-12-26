#include "utils.h"

#define MAXMESG   2048
#define UDP_CMD   "udpquit\n"

int main (int argc, char **argv) {

  if (argc != 1) {
    /* usage: ./dg_echo <sockfd> */
    exit(EXIT_FAILURE);
  }

  int   n, clilen;
  char  mesg[MAXMESG];

  struct sockaddr_in pcli_addr;
  struct sockaddr_in first_client;
  bzero(&pcli_addr, sizeof(struct sockaddr_in));
  clilen = sizeof(struct sockaddr_in);
  int   msgcnt = 0;

  for (;;) {
    n = recvfrom(0, mesg, MAXMESG, 0, (struct sockaddr *) &pcli_addr, (socklen_t *) &clilen);
    if (msgcnt == 0) {
      memcpy(&first_client, &pcli_addr, sizeof(first_client));
    } else {
      if (first_client.sin_port != pcli_addr.sin_port) {
        continue;
      }
    }
    if (n < 0) {
      exit(EXIT_FAILURE);
    }


    if (strncmp(mesg, UDP_CMD, 8) == 0) {
      // sendto(1, "closing\n", 9, 0, (struct sockaddr *) &pcli_addr, clilen);
      break;
    }

    if (sendto(1, mesg, n, 0, (struct sockaddr *) &pcli_addr, clilen) != n) {
      // perror("dg_echo: sendto error.");
      exit(EXIT_FAILURE);
    }
    msgcnt++;
  }

  close(0);
  close(1);
  close(2);
  exit(EXIT_SUCCESS);
}
