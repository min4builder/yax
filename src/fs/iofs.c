#define __YAX__
#define NDEBUG
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <codas/bit.h>
#include <yax/errorcodes.h>
#include <yax/lock.h>
#include <yax/openflags.h>
#include "fs/conn.h"
#include "fs/iofs.h"
#include "mem/malloc.h"
#include "multitask.h"
#include "pic.h"
#include "port.h"
#include "printk.h"

typedef struct {
	Conn c;
	int n;
} IoConn;

static Sem irqsem[16];

void iofsinterrupt(int n)
{
	printk("Interrupt ");
	iprintk(n);
	cprintk('\n');
	semsignal(&irqsem[n], 1);
	piceoi(n);
}

static Dev ddev;
static Dev idev;

Conn *iofsnew(void)
{
	IoConn *rd = calloc(1, sizeof(*rd) + 1);
	char *name = calloc(1, 1);
	conninit((Conn *)rd, name, 0, &ddev, rd);
	return (Conn *) rd;
}

static void del(Conn *c)
{
	free(c);
}

static Conn *dup(Conn *c)
{
	IoConn *d = calloc(1, sizeof(*d));
	memcpy(d, c, sizeof(*d));
	return (Conn *) d;
}

static ssize_t dirpreadelem(size_t *rlen, void **buf, size_t *len, struct stat const *st, char const *name)
{
	size_t nlen = strlen(name);
	size_t clen = 2 + STATMSGSIZ + 2 + nlen;

	if(clen > *len)
		return 0; /* stop */

	PBIT16(*buf, clen);
	*(char **)buf += 2;
	YAXstat2msg(st, *buf, *len);
	*(char **)buf += STATMSGSIZ;
	PBIT16(*buf, nlen);
	*(char **)buf += 2;
	memcpy(*buf, name, nlen);
	*(char **)buf += nlen;

	*rlen += clen;
	*len -= clen;

	return 1; /* continue */
}

static long long dfn(Conn *c, int fn, int submsg, void *buf, size_t len, void *buf2, size_t len2, off_t off)
{
	switch(fn) {
	case MSREAD: {
		size_t rlen = 0;
		char name[2];
		struct stat st = { .st_mode = S_IFIFO | 0400 };
		int i;
		name[1] = '\0';
		for(i = ((IoConn *)c)->n; i < 0x10; i++) {
			st.st_ino = 1 + i;
			name[0] = "0123456789abcdef"[i];
			if(!dirpreadelem(&rlen, &buf, &len, &st, name))
				break;
		}
		return rlen;
	}
	case MSTAT: {
		struct stat st = { .st_ino = 0, .st_mode = S_IFDIR | 0555 };
		if(len < STATMSGSIZ)
			return -EINVAL;
		return YAXstat2msg(&st, buf, len);
	}
	case MWALK: {
		int n;
		const char *path = buf;
		if(len != 1)
			return -ENOENT;
		if(len2 != 8)
			return -EINVAL;

		if('0' <= path[0] && path[0] <= '9')
			n = path[0] - '0';
		else if('a' <= path[0] && path[0] <= 'f')
			n = path[0] + 10 - 'a';
		else
			return -ENOENT;

		((IoConn *)c)->n = n;
		PBIT64(buf2, 1 + n);
		c->dev = &idev;
		return 0;
	}
	case MOPEN: {
		if(submsg & (O_EXCL | O_WRONLY))
			return -EISDIR;
		return 0;
	}
	default:
		return -ENOSYS;
	}
	(void) off;
}

static Dev ddev = {
	del,
	dup,
	dfn
};

static long long ifn(Conn *c, int fn, int submsg, void *buf, size_t len, void *buf2, size_t len2, off_t off)
{
	switch(fn) {
	case MSREAD: {
		if(len < 4)
			return -1;
		semwait(&irqsem[((IoConn *)c)->n], 1);
		PBIT32(buf, 1);
		return 4;
	}
	case MSTAT: {
		struct stat st = { .st_mode = S_IFIFO | 0400 };
		int i = ((IoConn *)c)->n;
		st.st_ino = 1 + i;
		return YAXstat2msg(&st, buf, len);
	}
	case MOPEN: {
		if(submsg & (O_EXCL | O_TRUNC | O_WRONLY))
			return -EACCES;
		/* todo implement exclusiveness */
		return 0;
	}
	default:
		return -ENOSYS;
	}
	(void) buf2, (void) len2, (void) off;
}

static Dev idev = {
	del,
	dup,
	ifn
};

