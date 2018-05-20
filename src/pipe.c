#include <sys/types.h>
#include "conn.h"
#include "lock.h"
#include "malloc.h"
#include "multitask.h"
#include "pipe.h"
#include "stat.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void pipenew(Pipe *ps[2])
{
	Buf *a, *b;
	ps[0] = malloc(sizeof(Pipe));
	ps[1] = malloc(sizeof(Pipe));
	conninit((Conn *)ps[0], CONNPIPE, "");
	conninit((Conn *)ps[1], CONNPIPE, "");
	ps[0]->other = ps[1];
	ps[1]->other = ps[0];
	a = malloc(sizeof(Buf));
	a->begin = a->len = 0;
	b = malloc(sizeof(Buf));
	b->begin = b->len = 0;
	ps[0]->a = a;
	ps[0]->b = b;
	ps[1]->a = b;
	ps[1]->b = a;
}
void pipedel(Pipe *p)
{
	if(p->other) {
		p->other->other = 0;
		p->other->a = 0;
	}
	free(p->b);
	free(p);
}

ssize_t piperead(Pipe *p, void *buf, size_t len)
{
	size_t endlen;
	Buf *b = p->b;
	if(!b || (!p->other && b->len == 0))
		return 0;
	while(b->len == 0)
		procswitch();
	locklock(&b->l);
	len = MIN(len, b->len);
	endlen = MIN(b->begin + len, BUFLEN) - b->begin;
	memcpy(buf, &b->buf[b->begin], endlen);
	memcpy((char *)buf + endlen, b->buf, len - endlen);
	b->len -= len;
	lockunlock(&b->l);
	return len;
}
ssize_t pipewrite(Pipe *p, const void *buf, size_t totallen)
{
	size_t endlen, len = 0;
	Buf *a;
	while(totallen) {
		while((a = p->a) && a->len == BUFLEN)
			procswitch();
		if(!a)
			return 0;
		locklock(&a->l);
		totallen -= len;
		len = MIN(totallen, BUFLEN - a->len);
		endlen = MIN((a->begin + a->len) % BUFLEN + len, BUFLEN) - (a->begin + a->len) % BUFLEN;
		memcpy(&a->buf[(a->begin + a->len) % BUFLEN], buf, endlen);
		memcpy(a->buf, (char *)buf + endlen, len - endlen);
		a->len += len;
		buf = (char *)buf + len;
		lockunlock(&a->l);
	}
	return len;
}

ssize_t pipestat(Pipe *p, void *buf, size_t len)
{
	Dir d = { { 0x40, 0, 0 }, 0x40000180, 0, 0, 0, "", "", "", "" };
	if(p->b)
		d.length = p->b->len;
	return convD2M(&d, buf, len);
}
ssize_t pipewstat(Pipe *p, const void *buf, size_t len)
{
	(void) p, (void) buf, (void) len;
	return -1;
}

