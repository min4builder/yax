#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stdint.h>
#include <sys/types.h>

void _exits(const char *);
void exits(const char *);
pid_t rfork(int);
int exec(const char *, const char *, const char *);

int mkmnt(int *);

void *mmap(void *, size_t, int, int, int, off_t);
int munmap(void *, size_t);

void (*notify(void (*)(void *, const char *)))(void *, const char *);
void __iopl(void);
void noted(int);

void sleep(clock_t);
void alarm(clock_t);

int open(const char *, int fl, ...);
void close(int);

ssize_t pread(int, void *, size_t, off_t);
ssize_t pwrite(int, const void *, size_t, off_t);
ssize_t read(int, void *, size_t);
ssize_t write(int, const void *, size_t);
off_t seek(int, off_t, int);
int dup2(int, int);
/* poll */
int chdir(const char *);

int pipe(int[2]);
int __mountfd(const char *, int, int fl);
ssize_t fd2path(int, char *, size_t);

ssize_t stat(const char *, void *, size_t);
ssize_t fstat(int, void *, size_t);
ssize_t wstat(const char *, const void *, size_t);
ssize_t fwstat(int, const void *, size_t);

int __getprintk(void);

#endif /* _SYSCALL_H */

