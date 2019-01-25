#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <yax/mapflags.h>
#include <yax/nflags.h>
#include <yax/rfflags.h>

#define RFMAX 63
#define MAP_MAX 31
#define PROT_MAX 7

void sys_exits(const char *);
pid_t sys_rfork(enum rfflags);

int sys_exec(const char *, const char *, const char *);

int sys_mkmnt(int *);

void *sys_mmap(void *, size_t, enum mapprot, enum mapflags, uint32_t, off_t);
int sys_munmap(void *, size_t);

void (*sys_notify(void (*)(void *, const char *)))(void *, const char *);
void sys_ioperm(size_t, size_t, int);
void sys_noted(enum nflags);

void sys_sleep(clock_t);
void sys_alarm(clock_t);

int sys_open(const char *, int, int);
void sys_close(int);
int sys_dup2(int, int);
int sys_poll(int *, unsigned int, clock_t); 
int sys_mount(const char *, int, int);
ssize_t sys_fd2path(int, char *, size_t);
void sys_chdir(const char *);
int sys_pipe(int[2]);

int sys_getprintk(void);

#endif /* _SYSCALL_H */

