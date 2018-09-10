#include <string.h>
#include <sys/types.h>
#include <yax/openflags.h>
#include "conn.h"
#include "lock.h"
#include "mem/malloc.h"
#include "multitask.h"
#include "pipe.h"
#include "stat.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define BUFLEN 4096

typedef struct {
	Lock l;
	Sem filled, empty;
	size_t begin, len;
	char buf[BUFLEN];
} Buf;

typedef struct Pipe Pipe;
struct Pipe {
	Conn c;
	Pipe *other;
	Buf *a, *b;
};

static Dev dev;

void pipenew(Conn *cs[2])
{
	Buf *a, *b;
	Pipe *ps[2];
	Qid q = { 0x44, 0, 0 }; /* QTAPPEND | QTTMP */
	ps[0] = malloc(sizeof(Pipe));
	ps[1] = malloc(sizeof(Pipe));
	q.path = 0;
	conninit((Conn *)ps[0], "", q, &dev, (void *)ps[0]);
	q.path = 1;
	conninit((Conn *)ps[1], "", q, &dev, (void *)ps[0]);
	ps[0]->other = ps[1];
	ps[1]->other = ps[0];
	a = malloc(sizeof(Buf));
	seminit(&a->filled, 0);
	seminit(&a->empty, BUFLEN);
	a->begin = a->len = 0;
	b = malloc(sizeof(Buf));
	seminit(&b->filled, 0);
	seminit(&b->empty, BUFLEN);
	b->begin = b->len = 0;
	ps[0]->a = a;
	ps[0]->b = b;
	ps[1]->a = b;
	ps[1]->b = a;
	cs[0] = (Conn *) ps[0];
	cs[1] = (Conn *) ps[1];
}

static void del(Conn *c)
{
	Pipe *p = (Pipe *) c;
	if(p->other) {
		p->other->other = 0;
		p->other->a = 0;
	}
	/* This is the only point where inconsistency is tolerated, and it is
	 * checked by both piperead and pipewrite to notice hangup */
	semsignal(&p->b->empty, BUFLEN);
	semsignal(&p->b->filled, BUFLEN);
	free(p->b);
	free(p);
}
static Conn *dup(Conn *c)
{
	ref(c);
	return c;
}

static ssize_t read(Conn *c, void *buf, size_t len)
{
	Pipe *p = (Pipe *) c;
	size_t endlen;
	Buf *b = p->b;
	semwait(&b->filled, 1);
	if(!p->other)
		return 0; /* read from closed pipe */
	ATOMIC(&b->l) {
		len = MIN(len, b->len);
		/* this is ok, won't block */
		semwait(&b->filled, len - 1);
		endlen = MIN(b->begin + len, BUFLEN) - b->begin;
		memcpy(buf, &b->buf[b->begin], endlen);
		memcpy((char *)buf + endlen, b->buf, len - endlen);
		b->len -= len;
		b->begin = (b->begin + len) % BUFLEN;
	}
	semsignal(&b->empty, len);
	return len;
}
static ssize_t pread(Conn *c, void *buf, size_t len, off_t off)
{
	(void) off;
	return read(c, buf, len);
}
static ssize_t write(Conn *c, const void *buf, size_t totallen)
{
	Pipe *p = (Pipe *) c;
	size_t endlen, len = 0;
	Buf *a = p->a;
	if(!a)
		return 0; /* write on closed pipe */
	while(totallen) {
		totallen -= len;
		semwait(&a->empty, 1);
		if(!p->other)
			return 0; /* write on closed pipe */
		ATOMIC(&a->l) {
			len = MIN(totallen, BUFLEN - a->len);
			/* this is ok, won't block */
			semwait(&a->empty, len - 1);
			endlen = MIN((a->begin + a->len) % BUFLEN + len, BUFLEN) - (a->begin + a->len) % BUFLEN;
			memcpy(&a->buf[(a->begin + a->len) % BUFLEN], buf, endlen);
			memcpy(a->buf, (char *)buf + endlen, len - endlen);
			a->len += len;
		}
		semsignal(&a->filled, len);
		buf = (char *)buf + len;
	}
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
	Dir d = { { 0x44, 0, 0 }, 0x44000180, 0, 0, 0, "", "", "", "" };
	d.qid = c->qid;
	if(((Pipe *) c)->b)
		d.length = ((Pipe *) c)->b->len;
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
static int open(Conn *c, int flags, int mode)
{
	if(flags & (OEXCL | OTRUNC))
		return -1;
	(void) c, (void) mode;
	return 0;
}

static Dev dev = {
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

