#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>
#include <sys/types.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

extern char **environ;

int close(int);
int dup2(int, int);
off_t lseek(int, off_t, int);
ssize_t read(int, void *, size_t);
ssize_t write(int, const void *, size_t);
int execve(const char *, const char *[], const char *[]);

void _exit(int);
void exit(int);

pid_t rfork(int);
int exec(const char *, const char *, const char *);
void exits(const char *);
void _exits(const char *);

#endif /* _UNISTD_H */

