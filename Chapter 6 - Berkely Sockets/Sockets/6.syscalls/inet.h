#ifndef INET_H
#define INET_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>

/*
 * Arbitraty port selection. The selected port must be greater than 1023, should be greater than 5000.
*/
#define   SERV_UDP_PORT   6969
#define   SERV_TCP_PORT   6969
/*
 * Hardcoding the server address for now, will observe better options later in the chapter...
*/
#define   SERV_HOST_ADDR  "192.168.1.71"    /* host addr for server */

char *pname;

#endif 
