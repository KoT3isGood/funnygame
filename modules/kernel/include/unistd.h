#ifndef _UNISTD_H
#define _UNISTD_H

#include "sys/types.h"

#define NULL 0

#define F_OK 0 
#define R_OK 1
#define W_OK 2
#define X_OK 7

#define SEEK_SET 0 
#define SEEK_CUR 1
#define SEEK_END 2

pid_t fork(void);
int execve(const char * filename, char ** argv, char ** envp);
int execv(const char * pathname, char ** argv);
int execvp(const char * file, char ** argv);
int execl(const char * pathname, char * arg0, ...);
int execlp(const char * file, char * arg0, ...);
int execle(const char * pathname, char * arg0, ...);
pid_t wait(int* stat_loc);
pid_t waitpid(pid_t pid, int* stat_loc, int options);


#endif
