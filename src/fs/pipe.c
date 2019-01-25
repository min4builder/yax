#define __YAX__
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <yax/lock.h>
#include <yax/openflags.h>
#include "fs/conn.h"
#include "fs/pipe.h"
#include "mem/malloc.h"
#include "multitask.h"

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
	ps[0] = malloc(sizeof(Pipe));
	ps[1] = malloc(sizeof(Pipe));
	conninit((Conn *) ps[0], "", 0, &dev, (void *)ps[0]);
	conninit((Conn *) ps[1], "", 1, &dev, (void *)ps[0]);
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

static long long fn(Conn *c, int fn, int submsg, void *buf, size_t len, void *buf2, size_t len2, off_t off)
{
	switch(fn) {
	case MSREAD: {
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
	case MSWRITE: {
		Pipe *p = (Pipe *) c;
		size_t endlen, clen = 0;
		Buf *a = p->a;
		if(!a)
			return 0; /* write on closed pipe */
		while(len) {
			len -= clen;
			semwait(&a->empty, 1);
			if(!p->other)
				return 0; /* write on closed pipe */
			ATOMIC(&a->l) {
				clen = MIN(len, BUFLEN - a->len);
				/* this is ok, won't block */
				semwait(&a->empty, clen - 1);
				endlen = MIN((a->begin + a->len) % BUFLEN + clen, BUFLEN) - (a->begin + a->len) % BUFLEN;
				memcpy(&a->buf[(a->begin + a->len) % BUFLEN], buf, endlen);
				memcpy(a->buf, (char *)buf + endlen, clen - endlen);
				a->len += clen;
			}
			semsignal(&a->filled, clen);
			buf = (char *)buf + clen;
		}
		return clen;
	}
	case MSTAT: {
		struct stat st = { .st_mode = S_IFIFO | 0600 };
		st.st_ino = c->ino;
		if(((Pipe *) c)->b)
			st.st_size = ((Pipe *) c)->b->len;
		return YAXstat2msg(&st, buf, len);
	}
	case MOPEN: {
		if(submsg & (O_EXCL | O_TRUNC))
			return -EACCES;
		return 0;
	}
	default:
		return -ENOSYS;
	}
	(void) buf2, (void) len2, (void) off;
}

static Dev dev = {
	del,
	dup,
	fn
};

