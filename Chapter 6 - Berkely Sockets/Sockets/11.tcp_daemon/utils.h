#ifndef UTILS_H
#define UTILS_H

#define LOG_MSG_LEN   128

int   readline  (register int fd, register char *ptr, register int maxlen);

void  str_echo  (int sockfd, int logfd);

int   writen    (register int fd, register char *ptr, register int nbytes);

int   write_log (int logfd, const char *logmsg);

#endif  /* UTILS_H */
