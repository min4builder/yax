#include <sys/types.h>
#include <yax/bit.h>
#include <yax/errorcodes.h>
#include <yax/openflags.h>
#include <yax/stat.h>
#include "conn.h"
#include "iofs.h"
#include "libk.h"
#include "lock.h"
#include "mem/malloc.h"
#include "multitask.h"
#include "pic.h"
#include "port.h"

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
	Qid qid = { 0x84, 0, 0 };
	conninit((Conn *)rd, name, qid, &rdev, rd);
	return (Conn *) rd;
}

static void del(Conn *c)
{
	free(c->name);
	free(c);
}

static Conn *iodup(Conn *c, const char *name)
{
	IoConn *d = calloc(1, sizeof(*d));
	memcpy(d, c, sizeof(*d));
	c->name = (char *) name;
	return (Conn *) d;
}

static Conn *dirdup(Conn *c, const char *name)
{
	DirConn *d = calloc(1, sizeof(*d));
	memcpy(d, c, sizeof(*d));
	c->name = (char *) name;
	return (Conn *) d;
}

static off_t pipeseek(Conn *c, off_t off, int whence)
{
	(void) c, (void) off, (void) whence;
	return -ESPIPE;
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

static off_t dirseek(Conn *c, off_t off, int whence)
{
	if(whence == 1)
		off += ((DirConn *) c)->off;
	else if(whence == 2)
		return -EINVAL;
	if(off != 0 && off != ((DirConn *) c)->off)
		return -EINVAL;
	return (((DirConn *) c)->off = off);
}

static ssize_t errwrite(Conn *c, const void *buf, size_t len)
{
	(void) c, (void) buf, (void) len;
	return -EACCES;
}
static ssize_t errpwrite(Conn *c, const void *buf, size_t len, off_t off)
{
	(void) off;
	return errwrite(c, buf, len);
}

static ssize_t errwstat(Conn *c, const void *buf, size_t len)
{
	(void) c, (void) buf, (void) len;
	return -EACCES;
}

static int diropen(Conn *c, int fl, int mode)
{
	if(fl & (OEXCL | OWRITE))
		return -EISDIR;
	(void) c, (void) mode;
	return 0;
}

static int fwalk(Conn *c, const char *path)
{
	(void) c, (void) path;
	return -ENOTDIR;
}

static ssize_t rpread(Conn *c, void *buf, size_t len, off_t off)
{
	static Dir ds[] = { 
		{ { 0x84, 1, 0 }, 0x8400016D, 0, 0, 0, "port", "", "", "" },
		{ { 0x84, 2, 0 }, 0x8400016D, 0, 0, 0, "irq", "", "", "" }
	};
	size_t tlen = 0;
	size_t rlen = 0;
	dirpreadelem(&rlen, &tlen, &buf, &len, off, &ds[0]);
	dirpreadelem(&rlen, &tlen, &buf, &len, off, &ds[1]);
	(void) c;
	return rlen;
}
static ssize_t rread(Conn *c, void *buf, size_t len)
{
	ssize_t ret = rpread(c, buf, len, ((DirConn *) c)->off);
	if(ret > 0)
		((DirConn *) c)->off += ret;
	return ret;
}

static ssize_t rstat(Conn *c, void *buf, size_t len)
{
	static Dir d = { { 0x84, 0, 0 }, 0x8400016D, 0, 0, 0, "", "", "", "" };
	(void) c;
	return convD2M(&d, buf, len);
}

static int rwalk(Conn *c, const char *path)
{
	if(strncmp(path, "port", 5) == 0 || strncmp(path, "port/", 5) == 0) {
		Qid qid = { 0x84, 1, 0 };
		c->qid = qid;
		c->dev = &pddev;
	} else if(strncmp(path, "irq", 4) == 0 || strncmp(path, "irq/", 4) == 0) {
		Qid qid = { 0x84, 2, 0 };
		c->qid = qid;
		c->dev = &iddev;
	} else
		return -ENOENT;
	return 0;
}

static Dev rdev = {
	del,
	dirdup,
	rpread,
	rread,
	errpwrite,
	errwrite,
	dirseek,
	rstat,
	errwstat,
	rwalk,
	diropen
};

static ssize_t pdpread(Conn *c, void *buf, size_t len, off_t off)
{
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
	(void) c;
	return rlen;
}
static ssize_t pdread(Conn *c, void *buf, size_t len)
{
	ssize_t ret = pdpread(c, buf, len, ((DirConn *) c)->off);
	if(ret > 0)
		((DirConn *) c)->off += ret;
	return ret;
}

static ssize_t pdstat(Conn *c, void *buf, size_t len)
{
	Dir d = { { 0x84, 1, 0 }, 0x8400016D, 0, 0, 0, "port", "", "", "" };
	(void) c;
	return convD2M(&d, buf, len);
}

static int pdwalk(Conn *c, const char *path)
{
	int n = 0;
	Qid qid = { 0x64, 0, 0 };
	if(path[0] == '\0' || path[1] == '\0' || path[2] == '\0' || path[3] == '\0')
		return -ENOENT;
	if(path[4] == '/')
		return -ENOTDIR;
	if(path[4] != '\0')
		return -ENOENT;
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
	c->qid = qid;
	c->dev = &pdev;
	return 0;
}

static Dev pddev = {
	del,
	dirdup,
	pdpread,
	pdread,
	errpwrite,
	errwrite,
	dirseek,
	pdstat,
	errwstat,
	pdwalk,
	diropen
};

static ssize_t idpread(Conn *c, void *buf, size_t len, off_t off)
{
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
	(void) c;
	return rlen;
}
static ssize_t idread(Conn *c, void *buf, size_t len)
{
	ssize_t ret = idpread(c, buf, len, ((DirConn *) c)->off);
	if(ret > 0)
		((DirConn *) c)->off += ret;
	return ret;
}

static ssize_t idstat(Conn *c, void *buf, size_t len)
{
	Dir d = { { 0x84, 1, 0 }, 0x8400016D, 0, 0, 0, "irq", "", "", "" };
	(void) c;
	return convD2M(&d, buf, len);
}

static int idwalk(Conn *c, const char *path)
{
	int n;
	Qid qid = { 0x64, 0, 0 };
	if(path[0] == '\0')
		return -ENOENT;
	if(path[1] == '/')
		return -ENOTDIR;
	if(path[1] != '\0')
		return -ENOENT;
	if('0' <= path[0] && path[0] <= '9')
		n = path[0] - '0';
	else if('a' <= path[0] && path[0] <= 'f')
		n = path[0] + 10 - 'a';
	else
		return -ENOENT;
	qid.path = 0x10003 + n;
	((IoConn *)c)->n = n;
	c->qid = qid;
	c->dev = &idev;
	return 0;
}

static Dev iddev = {
	del,
	dirdup,
	idpread,
	idread,
	errpwrite,
	errwrite,
	dirseek,
	idstat,
	errwstat,
	idwalk,
	diropen
};

static ssize_t pread(Conn *c, void *buf, size_t len)
{
	int p = ((IoConn *) c)->n;
	if(len == 1)
		*(uint8_t *) buf = inb(p);
	else
		return -EIO;
	return len;
}
static ssize_t ppread(Conn *c, void *buf, size_t len, off_t off)
{
	(void) off;
	return pread(c, buf, len);
}

static ssize_t pwrite(Conn *c, const void *buf, size_t len)
{
	int p = ((IoConn *)c)->n;
	if(len == 1)
		outb(p, *(uint8_t *)buf);
	else
		return -EIO;
	return len;
}
static ssize_t ppwrite(Conn *c, const void *buf, size_t len, off_t off)
{
	(void) off;
	return pwrite(c, buf, len);
}

static ssize_t pstat(Conn *c, void *buf, size_t len)
{
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

static int popen(Conn *c, int fl, int mode)
{
	/* TODO implement exclusiveness */
	if(fl & (OEXCL | OTRUNC))
		return -EACCES;
	(void) c, (void) mode;
	return 0;
}

static Dev pdev = {
	del,
	iodup,
	ppread,
	pread,
	ppwrite,
	pwrite,
	pipeseek,
	pstat,
	errwstat,
	fwalk,
	popen
};

static ssize_t iread(Conn *c, void *buf, size_t len)
{
	if(len < 4)
		return -1;
	semwait(&irqsem[((IoConn *)c)->n], 1);
	PBIT32(buf, 1);
	return 4;
}
static ssize_t ipread(Conn *c, void *buf, size_t len, off_t off)
{
	(void) off;
	return iread(c, buf, len);
}

static ssize_t istat(Conn *c, void *buf, size_t len)
{
	char name[2];
	Dir d = { { 0x64, 0, 0 }, 0x64000180, 0, 0, 0, 0, "", "", "" };
	int i = ((IoConn *)c)->n;
	name[0] = "0123456789abcdef"[i / 0x1000];
	name[1] = '\0';
	d.name = name;
	d.qid.path = 0x10003 + i;
	return convD2M(&d, buf, len);
}

static int iopen(Conn *c, int fl, int mode)
{
	if(fl & (OEXCL | OTRUNC | OWRITE))
		return -EACCES;
	/* todo implement exclusiveness */
	(void) c, (void) mode;
	return 0;
}

static Dev idev = {
	del,
	iodup,
	ipread,
	iread,
	errpwrite,
	errwrite,
	pipeseek,
	istat,
	errwstat,
	fwalk,
	iopen
};

