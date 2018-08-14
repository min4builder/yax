#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <yax/mapflags.h>
#include <yax/nflags.h>
#include <yax/rfflags.h>
#include "arch.h"
#include "libk.h"
#include "virtmman.h"

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
void sys_noted(enum nflags);

void sys_sleep(clock_t);
void sys_alarm(clock_t);

int sys_open(const char *, int, int);
void sys_close(int);
ssize_t sys_pread(int, void *, size_t, off_t);
ssize_t sys_pwrite(int, const void *, size_t, off_t);
int sys_dup2(int, int);
int sys_poll(int *, unsigned int, clock_t); 
void sys_mount(const char *, int, int);
ssize_t sys_fd2path(int, char *, size_t);
void sys_chdir(const char *);
int sys_pipe(int[2]);
ssize_t sys_read(int, void *, size_t);
ssize_t sys_write(int, const void *, size_t);
off_t sys_seek(int, off_t, int);
ssize_t sys_stat(const char *, void *, size_t);
ssize_t sys_fstat(int, void *, size_t);
ssize_t sys_wstat(const char *, const void *, size_t);
ssize_t sys_fwstat(int, const void *, size_t);

int sys_getprintk(void);

#endif /* _SYSCALL_H */

