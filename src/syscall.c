#define NDEBUG
#define __YAX__
#include <stdint.h>
#include <string.h>
#include <yax/errorcodes.h>
#include <yax/mapflags.h>
#include <yax/openflags.h>
#include <yax/rfflags.h>
#include "boot.h"
#include "conn.h"
#include "exec.h"
#include "fds.h"
#include "mem/malloc.h"
#include "mem/usrboundary.h"
#include "mem/virt.h"
#include "mnt.h"
#include "multitask.h"
#include "name.h"
#include "pipe.h"
#include "printk.h"
#include "printkfs.h"
#include "syscall.h"
#include "sysentry.h"

/* all functions in here are simple wrappers around internal kernel functions
 * they simply verify their arguments, print some debugging stuff and call
 * their internal equivalents */

void sys_exits(const char *s)
{
	(iprintk)(curproc->pid);
	(printk)(": exits(\"");
	if(s && !verusrstr(s, PROT_READ)) {
		(printk)("<invalid>\")\n");
		procexits("");
	}
	if(s == 0)
		s = "";
	(printk)(s);
	(printk)("\")\n");
	procexits(s);
}

pid_t sys_rfork(enum rfflags flags)
{
	pid_t pid;
	if(flags > RFMAX) {
		printk("Out of range!\n");
		return (pid_t) -EINVAL;
	}
	if(!(flags & RFPROC)) {
		printk("NO RFPROC\n");
		return (pid_t) -ENOTSUP;
	}
	pid = procrfork(flags);
	iprintk(curproc->pid);
	printk(": rfork(");
	if(flags & RFPROC)
		printk("|RFPROC");
	if(flags & RFNOWAIT)
		printk("|RFNOWAIT");
	if(flags & RFNAMEG)
		printk("|RFNAMEG");
	if(flags & RFMEM)
		printk("|RFMEM");
	if(flags & RFFDG)
		printk("|RFFDG");
	printk(") = ");
	uiprintk(pid);
	cprintk('\n');
	return pid;
}

int sys_exec(const char *name, const char *argv, const char *envp)
{
	uint32_t ret;
	Conn *c;
	void *entryp;
	iprintk(curproc->pid);
	printk(": exec(");
	if(!verusrstr(name, PROT_READ)) {
		printk("<invalid>) = -EINVAL;\n");
		return -EINVAL;
	}
	cprintk('"');
	printk(name);
	printk("\", ");
	if(!verusrstr(argv, PROT_READ) || strlen(argv) > 1024) {
		printk("<invalid>) = -EINVAL;\n");
		return -EINVAL;
	}
	cprintk('"');
	printk(argv);
	printk("\", ");
	if(!verusrstr(envp, PROT_READ) || strlen(envp) > 1024) {
		printk("<invalid>) = -EINVAL;\n");
		return -EINVAL;
	}
	cprintk('"');
	printk(envp);
	printk("\") = ");
	c = vfsopen(name, OREAD | OEXEC, 0);
	if(PTRERR(c)) {
		iprintk(PTR2ERR(c));
		printk(";\n");
		return PTR2ERR(c);
	}
	void *newargv = malloc(strlen(argv) + 1);
	memcpy(newargv, argv, strlen(argv) + 1);
	void *newenvp = malloc(strlen(envp) + 1);
	memcpy(newenvp, envp, strlen(envp) + 1);
	ret = exec(c, &entryp, newargv, newenvp);
	uxprintk(ret);
	printk(";\n");
	unref(c);
	if(PTRERR(ret))
		goto bail;
	free(newargv);
	free(newenvp);
	usermode(entryp, (void *) ret);
bail:
	free(newargv);
	free(newenvp);
	return ret;
}

int sys_mkmnt(int *master)
{
	Conn *root, *m;
	if(!verusrptr(master, sizeof(*master), PROT_WRITE))
		return -EINVAL;
	root = mntnew(&m);
	if(!root)
		return -EIO;
	*master = fdalloc(m);
	return fdalloc(root);
}

void *sys_mmap(void *addr, size_t len, enum mapprot prot, enum mapflags flags, uint32_t fd, off_t off)
{
	void *ret;
	Conn *c = 0;
	iprintk(curproc->pid);
	printk(": mmap(");
	uxprintk((uint32_t)addr);
	printk(", ");
	uxprintk(len);
	printk(", ");
	uxprintk(prot);
	printk(", ");
	uxprintk(flags);
	printk(", ");
	uxprintk(fd);
	printk(", ");
	uxprintk(off);
	printk(") = ");
	if(flags > MAP_MAX || prot > PROT_MAX
	|| (flags & MAP_FIXED && ((unsigned int) addr % PGLEN || addr == 0))) {
		printk("Out of range!\n");
		return (void *) -EINVAL;
	} else if(flags & MAP_FIXED && (uint8_t *) addr + len > VIRT(0) ) {
		printk("Can't map over kernel!\n");
		return (void *) -EINVAL;
	}
	if(!(flags & (MAP_ANONYMOUS | MAP_PHYS))) {
		c = FD2CONN(fd);
		if(!c) {
			printk("-EINVAL;\n");
			return ERR2PTR(-EINVAL);
		}
	}
	if(flags & MAP_ANONYMOUS)
		ret = vpgumap(addr, len, prot, flags);
	else if(flags & MAP_PHYS)
		ret = vpgpmap(addr, len, prot, flags, off);
	else
		ret = vpgfmap(addr, len, prot, flags, c, off);
	uxprintk((uintptr_t) ret);
	printk(";\n");
	return ret;
}

int sys_munmap(void *addr, size_t len)
{
	iprintk(curproc->pid);
	printk(": munmap(");
	uxprintk((uintptr_t)addr);
	printk(", ");
	uxprintk(len);
	printk(");\n");
	if((unsigned int) addr % PGLEN || addr == 0 || len == 0 || (uint8_t *) addr + len > VIRT(0)) {
		printk("Out of range!\n");
		return -EINVAL;
	}
	vpgunmap(addr, len);
	return 0;
}

void (*sys_notify(void (*f)(void *, const char *)))(void *, const char *)
{
	iprintk(curproc->pid);
	printk(": notify(");
	uxprintk((uintptr_t) f);
	printk(");\n");
	return proconnotify(f);
}

void sys_noted(enum nflags f)
{
	iprintk(curproc->pid);
	printk(": noted()\n");
	procnoted(f);
}

void sys_sleep(clock_t time)
{
	iprintk(curproc->pid);
	printk(": sleep(");
	uiprintk(time);
	printk(");\n");
	procsleep(time);
}

void sys_alarm(clock_t time)
{
	iprintk(curproc->pid);
	printk(": alarm(");
	uiprintk(time);
	printk(");\n");
	procalarm(time);
}

int sys_open(const char *name, int fl, int mode)
{
	Conn *c;
	int fd;
	iprintk(curproc->pid);
	printk(": open(\"");
	if(!verusrstr(name, PROT_READ)) {
		printk("<invalid>\", ");
		uiprintk(fl);
		if(fl & OCREAT) {
			printk(", ");
			uiprintk(mode);
		}
		printk(") = -EINVAL;\n");
		return -EINVAL;
	}
	printk(name);
	printk("\", ");
	uiprintk(fl);
	if(fl & OCREAT) {
		printk(", ");
		uiprintk(mode);
	}
	printk(") = ");
	c = vfsopen(name, fl, mode);
	if(PTRERR(c)) {
		iprintk(PTR2ERR(c));
		printk(";\n");
		return PTR2ERR(c);
	}
	fd = fdalloc(c);
	iprintk(fd);
	printk(";\n");
	return fd;
}
void sys_close(int fd)
{
	iprintk(curproc->pid);
	printk(": close(");
	uiprintk(fd);
	printk(");\n");
	fddealloc(fd);
}

ssize_t sys_pread(int fd, void *buf, size_t len, off_t off)
{
	ssize_t ret;
	Conn *c;
	iprintk(curproc->pid);
	printk(": pread(");
	uiprintk(fd);
	printk(", 0x");
	uxprintk((unsigned int)buf);
	if(!verusrptr(buf, len, PROT_WRITE)) {
		printk(" <invalid>, ");
		uiprintk(len);
		printk(", ");
		uiprintk(off);
		printk(") = -EINVAL;\n");
		return -EINVAL;
	}
	printk(", ");
	uiprintk(len);
	printk(", ");
	uiprintk(off);
	printk(") = ");
	c = FD2CONN(fd);
	if(!c) {
		printk("-EINVAL;\n");
		return -EINVAL;
	}
	ret = connpread(c, buf, len, off);
	iprintk(ret);
	if(ret > 0) {
		printk(" \"");
		nprintk(ret, buf);
		cprintk('"');
	}
	printk(";\n");
	return ret;
}

ssize_t sys_pwrite(int fd, const void *buf, size_t len, off_t off)
{
	ssize_t ret;
	Conn *c;
	iprintk(curproc->pid);
	printk(": pwrite(");
	uiprintk(fd);
	printk(", 0x");
	uxprintk((unsigned int)buf);
	if(!verusrptr(buf, len, PROT_READ)) {
		printk(" <invalid>, ");
		uiprintk(len);
		printk(", ");
		uiprintk(off);
		printk(") = -EINVAL;\n");
		return -EINVAL;
	}
	cprintk('"');
	nprintk(len, buf);
	printk("\", ");
	uiprintk(len);
	printk(", ");
	uiprintk(off);
	printk(") = ");
	c = FD2CONN(fd);
	if(!c) {
		printk("-EINVAL;\n");
		return -EINVAL;
	}
	ret = connpwrite(c, buf, len, off);
	iprintk(ret);
	printk(";\n");
	return ret;
}

ssize_t sys_read(int fd, void *buf, size_t len)
{
	ssize_t ret;
	Conn *c;
	iprintk(curproc->pid);
	printk(": read(");
	uiprintk(fd);
	printk(", 0x");
	uxprintk((unsigned int)buf);
	if(!verusrptr(buf, len, PROT_WRITE)) {
		printk(" <invalid>, ");
		uiprintk(len);
		printk(") = -EINVAL;\n");
		return -EINVAL;
	}
	printk(", ");
	uiprintk(len);
	printk(") = ");
	c = FD2CONN(fd);
	if(!c) {
		printk("-EINVAL;\n");
		return -EINVAL;
	}
	ret = connread(c, buf, len);
	iprintk(ret);
	if(ret > 0) {
		printk(" \"");
		nprintk(ret, buf);
		cprintk('"');
	}
	printk(";\n");
	return ret;
}

ssize_t sys_write(int fd, const void *buf, size_t len)
{
	ssize_t ret;
	Conn *c;
	iprintk(curproc->pid);
	printk(": write(");
	uiprintk(fd);
	printk(", 0x");
	uxprintk((unsigned int) buf);
	if(!verusrptr(buf, len, PROT_READ)) {
		printk(" <invalid>, ");
		uiprintk(len);
		printk(") = -EINVAL;\n");
		return -EINVAL;
	}
	cprintk('"');
	nprintk(len, buf);
	printk("\", ");
	uiprintk(len);
	printk(") = ");
	c = FD2CONN(fd);
	if(!c) {
		printk("-EINVAL;\n");
		return -EINVAL;
	}
	ret = connwrite(c, buf, len);
	iprintk(ret);
	printk(";\n");
	return ret;
}

off_t sys_seek(int fd, off_t off, int whence)
{
	off_t ret;
	Conn *c;
	iprintk(curproc->pid);
	printk(": seek(");
	uiprintk(fd);
	printk(", ");
	uiprintk(off);
	printk(", ");
	uiprintk(whence);
	printk(") = ");
	c = FD2CONN(fd);
	if(!c) {
		printk("-EINVAL;\n");
		return -EINVAL;
	}
	ret = connseek(c, off, whence);
	iprintk(ret);
	printk(";\n");
	return ret;
}

int sys_dup2(int from, int to)
{
	Conn *c = FD2CONN(from);
	if(!c)
		return -EINVAL;
	if(to < 0)
		return fdalloc(c);
	return FDSET(to, c);
}

void sys_chdir(const char *name)
{
	iprintk(curproc->pid);
	printk(": chdir(");
	if(!verusrstr(name, PROT_READ)) {
		printk("<invalid>);\n");
		return;
	}
	printk(name);
	vfschdir(name);
	printk(");\n");
}

int sys_mountfd(const char *from, int to, int fl)
{
	Conn *c;
	int res;
	iprintk(curproc->pid);
	printk(": mount(\"");
	if(!verusrstr(from, PROT_READ)) {
		printk("<invalid>\", ");
		uiprintk(to);
		printk(", ");
		uiprintk(fl);
		printk(") = -EINVAL;\n");
		return -EINVAL;
	}
	printk(from);
	printk("\", ");
	uiprintk(to);
	printk(", ");
	uiprintk(fl);
	printk(") = ");
	c = FD2CONN(to);
	if(!c) {
		printk("-EINVAL;\n");
		return -EINVAL;
	}
	res = vfsmount(from, c, fl);
	iprintk(res);
	printk(";\n");
	return res;
}

int sys_pipe(int fds[2])
{
	Conn *ps[2];
	if(!verusrptr(fds, sizeof(int[2]), PROT_WRITE))
		return -EINVAL;
	iprintk(curproc->pid);
	printk(": pipe([");
	pipenew(ps);
	fds[0] = fdalloc(ps[0]);
	fds[1] = fdalloc(ps[1]);
	iprintk(fds[0]);
	printk(", ");
	iprintk(fds[1]);
	printk("]);\n");
	return 0;
}

ssize_t sys_fd2path(int fd, char *buf, size_t len)
{
	Conn *c = FD2CONN(fd);
	if(!c || !verusrptr(buf, len, PROT_WRITE))
		return -EINVAL;
	return strlcpy(buf, c->name->s, len);
}

ssize_t sys_fstat(int fd, void *buf, size_t len)
{
	ssize_t ret;
	Conn *c;
	iprintk(curproc->pid);
	printk(": fstat(");
	uiprintk(fd);
	printk(", 0x");
	uxprintk((unsigned int) buf);
	if(!verusrptr(buf, len, PROT_WRITE)) {
		printk(" <invalid>, ");
		uiprintk(len);
		printk(") = -EINVAL;\n");
		return -EINVAL;
	}
	cprintk('"');
	nprintk(len, buf);
	printk("\", ");
	uiprintk(len);
	printk(") = ");
	c = FD2CONN(fd);
	if(!c) {
		printk("-EINVAL;\n");
		return -EINVAL;
	}
	ret = connstat(c, buf, len);
	iprintk(ret);
	printk(";\n");
	return ret;
}

ssize_t sys_fwstat(int fd, const void *buf, size_t len)
{
	ssize_t ret;
	Conn *c;
	iprintk(curproc->pid);
	printk(": fwstat(");
	uiprintk(fd);
	printk(", 0x");
	uxprintk((unsigned int) buf);
	if(!verusrptr(buf, len, PROT_READ)) {
		printk(" <invalid>, ");
		uiprintk(len);
		printk(") = -EINVAL;\n");
		return -EINVAL;
	}
	cprintk('"');
	nprintk(len, buf);
	printk("\", ");
	uiprintk(len);
	printk(") = ");
	c = FD2CONN(fd);
	if(!c) {
		printk("-EINVAL;\n");
		return -EINVAL;
	}
	ret = connwstat(c, buf, len);
	iprintk(ret);
	printk(";\n");
	return ret;
}

int sys_getprintk(void)
{
	Conn *c;
	int fd;
	iprintk(curproc->pid);
	printk(": getprintk() = ");
	c = printkfsnew();
	if(!c) {
		printk("-1;\n");
		return -1;
	}
	fd = fdalloc(c);
	iprintk(fd);
	printk(";\n");
	return fd;
}

