#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stdint.h>
#include <sys/types.h>

void _exits(const char *);
pid_t rfork(int);
void test(int);

void *mmap(void *, size_t, int, int, int, off_t);
int munmap(void *, size_t);

void (*notify(void (*)(void *, const char *)))(void *, const char *);
void noted(int);

void sleep(clock_t);
void alarm(clock_t);

void close(int);

ssize_t pread(int, void *, size_t, off_t);
ssize_t pwrite(int, const void *, size_t, off_t);
ssize_t read(int, void *, size_t);
ssize_t write(int, const void *, size_t);

int pipe(int[2]);

ssize_t fstat(int, void *, size_t);
ssize_t fwstat(int, const void *, size_t);

void __printk(const char *);
void __cprintk(char);

#endif /* _SYSCALL_H */

