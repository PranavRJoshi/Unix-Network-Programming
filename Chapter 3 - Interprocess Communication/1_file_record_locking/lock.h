#ifndef LOCK_H
#define LOCK_H

/* 1 - Simple lock and unlock using lockf library function */
void my_lock (int fd);

void my_unlock (int fd);

/* 2 - Simple lock and unlock using flock system call */
void my_sys_call_lock (int fd);

void my_sys_call_unlock (int fd);

/* 3 - Simple lock and unlock using link system call */
void my_link_lock (int fd);

void my_link_unlock (int fd);

/* 4 - Simple lock and unlock which uses feature of the creat system call */
void my_creat_lock (int fd);

void my_creat_unlock (int fd);

/* 5 - Simple lock and unlock which uses feature of the open system call */
void my_open_lock (int fd);

void my_open_unlock (int fd);

#endif
