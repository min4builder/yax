#ifdef NDEBUG
#undef NDEBUG
#endif
#define __YAX__
#include <sys/types.h>
#include <yax/openflags.h>
#include "conn.h"
#include "mem/malloc.h"
#include "printk.h"
#include "printkfs.h"
#include "stat.h"

static Dev ops;

Conn *printkfsnew(void)
{
	Qid qid = { 0x44, 0, 0 };
	Conn *c = calloc(1, sizeof(*c));
	conninit(c, "/dev/printk", qid, &ops, 0);
	return c;
}

static void del(Conn *c)
{
	free(c);
}
static Conn *dup(Conn *c)
{
	ref(c);
	return c;
}

static ssize_t read(Conn *c, void *buf, size_t len)
{
	(void) c, (void) buf, (void) len;
	return 0;
}
static ssize_t pread(Conn *c, void *buf, size_t len, off_t off)
{
	(void) off;
	return read(c, buf, len);
}
static ssize_t write(Conn *c, const void *buf, size_t len)
{
	nprintk(len, (const char *) buf);
	(void) c;
	return len;
}
static ssize_t pwrite(Conn *c, const void *buf, size_t len, off_t off)
{
	(void) off;
	return write(c, buf, len);
}

static off_t seek(Conn *c, off_t off, int whence)
{
	(void) c, (void) off, (void) whence;
	return -ESPIPE;
}

static ssize_t stat(Conn *c, void *buf, size_t len)
{
	Dir d = { { 0x44, 0, 0 }, 0x44000100, 0, 0, 0, "", "", "", "" };
	(void) c;
	return convD2M(&d, buf, len);
}
static ssize_t wstat(Conn *c, const void *buf, size_t len)
{
	(void) c, (void) buf, (void) len;
	return -1;
}

static int walk(Conn *c, const char *path)
{
	(void) c, (void) path;
	return -1;
}
static int open(Conn *c, int fl, int mode)
{
	if(fl & (OEXCL | OTRUNC))
		return -1;
	(void) c, (void) mode;
	return 0;
}

static Dev ops = {
	del,
	dup,
	pread,
	read,
	pwrite,
	write,
	seek,
	stat,
	wstat,
	walk,
	open
};

