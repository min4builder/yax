#include <_yalc.h>

#if !defined(_UNISTD_H_POSIX) && _YALC_NEED_POSIX
#define _UNISTD_H_POSIX

#include <stdint.h>

#ifndef NULL
#define NULL _YalcNULL
#endif

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

_YALCDEFTYPE(size_t);
_YALCDEFTYPE(ssize_t);
_YALCDEFTYPE(uid_t);
_YALCDEFTYPE(gid_t);
_YALCDEFTYPE(off_t);
_YALCDEFTYPE(pid_t);

extern char **environ;

int close(int);
int dup2(int, int);
int execve(const char *, char *const[], char *const[]);
_Noreturn void _exit(int);
_Noreturn void exit(int);
pid_t fork(void);
off_t lseek(int, off_t, int);
ssize_t read(int, void *, size_t);
ssize_t write(int, const void *, size_t);

#endif /* _UNISTD_H_POSIX */

#if !defined(_UNISTD_H_YAX) && _YALC_NEED_YAX
#define _UNISTD_H_YAX

#include <yax/rfflags.h>

int exec(const char *, const char *, const char *);
_Noreturn void _exits(const char *);
_Noreturn void exits(const char *);
pid_t rfork(int);

#endif /* _UNISTD_H_YAX */

