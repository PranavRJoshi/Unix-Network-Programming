#ifndef IPC_H
#define IPC_H

#define   FIFO1     "/tmp/fifo.1"
#define   FIFO2     "/tmp/fifo.2"
#define   PERMS     0666

void client (int ipcreadfd, int ipcwritefd);

void server (int ipcreadfd, int ipcwritefd);

#endif
