#define __YAX__
#define NDEBUG
#include <string.h>
#include <sys/types.h>
#include <codas/bit.h>
#include <yax/errorcodes.h>
#include <yax/lock.h>
#include <yax/openflags.h>
#include <yax/stat.h>
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

typedef struct {
	Conn c;
	off_t off;
} DirConn;

static Sem irqsem[16];

void iofsinterrupt(int n)
{
	printk("Interrupt ");
	iprintk(n);
	cprintk('\n');
	semsignal(&irqsem[n], 1);
	piceoi(n);
}

static Dev rdev;
static Dev pddev;
static Dev iddev;
static Dev pdev;
static Dev idev;

Conn *iofsnew(void)
{
	DirConn *rd = calloc(1, sizeof(*rd) + 1);
	char *name = calloc(1, 1);
	Qid qid = { QTDIR | QTTMP, 0, 0 };
	conninit((Conn *)rd, name, qid, &rdev, rd);
	return (Conn *) rd;
}

static void del(Conn *c)
{
	free(c);
}

static Conn *iodup(Conn *c)
{
	IoConn *d = calloc(1, sizeof(*d));
	memcpy(d, c, sizeof(*d));
	return (Conn *) d;
}

static Conn *dirdup(Conn *c)
{
	DirConn *d = calloc(1, sizeof(*d));
	memcpy(d, c, sizeof(*d));
	return (Conn *) d;
}

static ssize_t dirpreadelem(size_t *rlen, size_t *tlen, void **buf, size_t *len, off_t off, Dir *d)
{
	size_t clen = convD2M(d, 0, 0);
	if(*tlen >= off) {
		if(clen > *len)
			return 0; /* stop */
		convD2M(d, *buf, *len);
		*rlen += clen;
		*(char **)buf += clen;
		*len -= clen;
	}
	*tlen += clen;
	return 1; /* continue */
}

static long long rfn(Conn *c, int fn, int submsg, void *buf, size_t len, void *buf2, size_t len2, off_t off)
{
	switch(fn) {
	case MPREAD: {
		static Dir ds[] = { 
			{ { 0x84, 1, 0 }, 0x8400016D, 0, 0, 0, "port", "", "", "" },
			{ { 0x84, 2, 0 }, 0x8400016D, 0, 0, 0, "irq", "", "", "" }
		};
		size_t tlen = 0;
		size_t rlen = 0;
		dirpreadelem(&rlen, &tlen, &buf, &len, off, &ds[0]);
		dirpreadelem(&rlen, &tlen, &buf, &len, off, &ds[1]);
		return rlen;
	}
	case MSTAT: {
		static Dir d = { { 0x84, 0, 0 }, 0x8400016D, 0, 0, 0, "", "", "", "" };
		return convD2M(&d, buf, len);
	}
	case MWALK: {
		if(len2 != 13)
			return -EINVAL;
		if(strncmp(buf, "port", len) == 0) {
			Qid qid = { 0x84, 1, 0 };
			PBIT8(buf2, qid.type);
			PBIT32((char *)buf2+1, qid.vers);
			PBIT64((char *)buf2+4, qid.path);
			c->dev = &pddev;
		} else if(strncmp(buf, "irq", len) == 0) {
			Qid qid = { 0x84, 2, 0 };
			PBIT8(buf2, qid.type);
			PBIT32((char *)buf2+1, qid.vers);
			PBIT64((char *)buf2+4, qid.path);
			c->dev = &iddev;
		} else
			return -ENOENT;
		return 0;
	}
	case MOPEN: {
		if(submsg & (O_EXCL | O_WRONLY))
			return -EISDIR;
		return 0;
	}
	default:
		return -EINVAL;
	}
}

static Dev rdev = {
	del,
	dirdup,
	rfn
};

static long long pdfn(Conn *c, int fn, int submsg, void *buf, size_t len, void *buf2, size_t len2, off_t off)
{
	switch(fn) {
	case MPREAD: {
		size_t tlen = 0;
		size_t rlen = 0;
		char name[5];
		Dir d = { { 0x64, 0, 0 }, 0x64000180, 0, 0, 0, 0, "", "", "" };
		int i;
		d.name = name;
		name[4] = '\0';
		for(i = 0; i < 0x10000; i++) {
			d.qid.path = 3 + i;
			name[0] = "0123456789abcdef"[i / 0x1000];
			name[1] = "0123456789abcdef"[(i / 0x100) % 0x10];
			name[2] = "0123456789abcdef"[(i / 0x10) % 0x10];
			name[3] = "0123456789abcdef"[i % 0x10];
			if(!dirpreadelem(&rlen, &tlen, &buf, &len, off, &d))
				break;
		}
		return rlen;
	}
	case MSTAT: {
		Dir d = { { 0x84, 1, 0 }, 0x8400016D, 0, 0, 0, "port", "", "", "" };
		return convD2M(&d, buf, len);
	}
	case MWALK: {
		int n = 0;
		Qid qid = { 0x64, 0, 0 };
		const char *path = buf;
		if(len != 4)
			return -ENOENT;
		if(len2 != 13)
			return -EINVAL;

		if('0' <= path[0] && path[0] <= '9')
			n += path[0] - '0';
		else if('a' <= path[0] && path[0] <= 'f')
			n += path[0] + 10 - 'a';
		else
			return -ENOENT;
		n *= 0x10;

		if('0' <= path[1] && path[1] <= '9')
			n += path[1] - '0';
		else if('a' <= path[1] && path[1] <= 'f')
			n += path[1] + 10 - 'a';
		else
			return -ENOENT;
		n *= 0x10;

		if('0' <= path[2] && path[2] <= '9')
			n += path[2] - '0';
		else if('a' <= path[2] && path[2] <= 'f')
			n += path[2] + 10 - 'a';
		else
			return -ENOENT;
		n *= 0x10;

		if('0' <= path[3] && path[3] <= '9')
			n += path[3] - '0';
		else if('a' <= path[3] && path[3] <= 'f')
			n += path[3] + 10 - 'a';
		else
			return -ENOENT;

		qid.path = 3 + n;
		((IoConn *) c)->n = n;
		PBIT8(buf2, qid.type);
		PBIT32((char *)buf2+1, qid.vers);
		PBIT64((char *)buf2+4, qid.path);
		c->dev = &pdev;
		return 0;
	}
	case MOPEN: {
		if(submsg & (O_EXCL | O_WRONLY))
			return -EISDIR;
		return 0;
	}
	default:
		return -EINVAL;
	}
}

static Dev pddev = {
	del,
	dirdup,
	pdfn
};

static long long idfn(Conn *c, int fn, int submsg, void *buf, size_t len, void *buf2, size_t len2, off_t off)
{
	switch(fn) {
	case MPREAD: {
		size_t tlen = 0;
		size_t rlen = 0;
		char name[2];
		Dir d = { { 0x64, 0, 0 }, 0x64000180, 0, 0, 0, 0, "", "", "" };
		int i;
		d.name = name;
		name[1] = '\0';
		for(i = 0; i < 0x10; i++) {
			d.qid.path = 0x10003 + i;
			name[0] = "0123456789abcdef"[i];
			if(!dirpreadelem(&rlen, &tlen, &buf, &len, off, &d))
				break;
		}
		return rlen;
	}
	case MSTAT: {
		Dir d = { { 0x84, 1, 0 }, 0x8400016D, 0, 0, 0, "irq", "", "", "" };
		return convD2M(&d, buf, len);
	}
	case MWALK: {
		int n;
		Qid qid = { 0x64, 0, 0 };
		const char *path = buf;
		if(len != 1)
			return -ENOENT;
		if(len2 != 13)
			return -EINVAL;

		if('0' <= path[0] && path[0] <= '9')
			n = path[0] - '0';
		else if('a' <= path[0] && path[0] <= 'f')
			n = path[0] + 10 - 'a';
		else
			return -ENOENT;

		qid.path = 0x10003 + n;
		((IoConn *)c)->n = n;
		PBIT8(buf2, qid.type);
		PBIT32((char *)buf2+1, qid.vers);
		PBIT64((char *)buf2+4, qid.path);
		c->dev = &idev;
		return 0;
	}
	case MOPEN: {
		if(submsg & (O_EXCL | O_WRONLY))
			return -EISDIR;
		return 0;
	}
	default:
		return -EINVAL;
	}
}

static Dev iddev = {
	del,
	dirdup,
	idfn
};

static long long pfn(Conn *c, int fn, int submsg, void *buf, size_t len, void *buf2, size_t len2, off_t off)
{
	switch(fn) {
	case MSREAD: {
		int p = ((IoConn *) c)->n;
		if(len == 1)
			*(uint8_t *) buf = inb(p);
		else
			return -EIO;
		return len;
	}
	case MSWRITE: {
		int p = ((IoConn *)c)->n;
		if(len == 1)
			outb(p, *(uint8_t *)buf);
		else
			return -EIO;
		return len;
	}
	case MSTAT: {
		char name[5];
		Dir d = { { 0x64, 0, 0 }, 0x64000180, 0, 0, 0, 0, "", "", "" };
		int i = ((IoConn *)c)->n;
		name[0] = "0123456789abcdef"[i / 0x1000];
		name[1] = "0123456789abcdef"[(i / 0x100) % 0x10];
		name[2] = "0123456789abcdef"[(i / 0x10) % 0x10];
		name[3] = "0123456789abcdef"[i % 0x10];
		name[4] = '\0';
		d.name = name;
		d.qid.path = 3 + i;
		return convD2M(&d, buf, len);
	}
	case MOPEN: {
		/* TODO implement exclusiveness */
		if(submsg & (O_EXCL | O_TRUNC))
			return -EACCES;
		return 0;
	}
	default:
		return -EINVAL;
	}
	(void) buf2, (void) len2, (void) off;
}

static Dev pdev = {
	del,
	iodup,
	pfn
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
		char name[2];
		Dir d = { { 0x64, 0, 0 }, 0x64000180, 0, 0, 0, 0, "", "", "" };
		int i = ((IoConn *)c)->n;
		name[0] = "0123456789abcdef"[i / 0x1000];
		name[1] = '\0';
		d.name = name;
		d.qid.path = 0x10003 + i;
		return convD2M(&d, buf, len);
	}
	case MOPEN: {
		if(submsg & (O_EXCL | O_TRUNC | O_WRONLY))
			return -EACCES;
		/* todo implement exclusiveness */
		return 0;
	}
	default:
		return -EINVAL;
	}
	(void) buf2, (void) len2, (void) off;
}

static Dev idev = {
	del,
	iodup,
	ifn
};

