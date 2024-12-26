#include "utils.h"

#define MAXMESG   2048
#define UDP_DIS   "udpdiscard\n"

int main (int argc, char **argv) {

  if (argc != 1) {
    /* usage: ./dg_echo <sockfd> */
    exit(EXIT_FAILURE);
  }

  int   n, clilen;
  char  mesg[MAXMESG];

  struct sockaddr_in pcli_addr;
  bzero(&pcli_addr, sizeof(struct sockaddr_in));
  struct sockaddr_in first_client;
  bzero(&first_client, sizeof(struct sockaddr_in));
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
    } else if (n == 0) {
      sendto(1, "closing\n", 9, 0, (struct sockaddr *) &pcli_addr, clilen);
    }

    if (strncmp(mesg, UDP_DIS, 11) == 0) {
      if (sendto(1, "closing\n", 9, 0, (struct sockaddr *) &pcli_addr, clilen) != 9) {
        exit(EXIT_FAILURE);
      }
      break;
    }

    /*
     * For some bizzare reason, now having this will break `dg_dis` executable, and the program won't end 
     * even if the terminating command is provided.
    */
    if (sendto(1, mesg, 0, 0, (struct sockaddr *) &pcli_addr, clilen) != 0) {
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

