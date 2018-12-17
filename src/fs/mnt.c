#define __YAX__
#include <string.h>
#include <codas/bit.h>
#include <yax/openflags.h>
#include <yax/stat.h>
#include "fs/conn.h"
#include "fs/mnt.h"
#include "mem/malloc.h"
#include "mem/usrboundary.h"
#include "multitask.h"

/* TODO gather supported messages */

typedef struct Msg Msg;
struct Msg {
	Msg *next;
	Sem done;
	int fn;
	int submsg;
	int fid;
	PgList *pl;
	void *buf;
	size_t len;
	PgList *pl2;
	void *buf2;
	size_t len2;
	off_t off;
	long long ret;
};

typedef struct {
	Conn c;
	Sem ready, hasmsg;
	off_t read;
	Msg *msg;
	Msg *msgs;
} Server;
typedef struct {
	Conn c;
	Server *s;
	int fid;
	Msg msg; /* we can only have a message at a time currently */
} Client;

static void sendmsg(Server *s, Msg *m)
{
	condwait(&s->ready);
	s->msg = m;
	condsignal(&s->hasmsg);
	condwait(&m->done);
}

static Dev sops;
static Dev cops;

Conn *mntnew(Conn **master, Qid rq)
{
	Server *s = calloc(1, sizeof(*s));
	Client *root = calloc(1, sizeof(*root));
	Qid qid = { 0x44, 0, 0 };
	if(!s || !root)
		return 0;
	conninit((Conn *)s, "", qid, &sops, s);
	seminit(&s->ready, 0);
	seminit(&s->hasmsg, 0);
	s->msgs = 0;
	s->read = 0;
	*master = (Conn *) s;
	conninit((Conn *)root, "", rq, &cops, s);
	root->s = s;
	root->fid = 0;
	ref(s); /* TODO every client has a reference; might not be a good idea */
	return (Conn *) root;
}

static void sdel(Conn *c)
{
	free(c);
}
static Conn *sdup(Conn *c)
{
	ref(c);
	return c;
}

static long long sfn(Conn *c, int fn, int submsg, void *buf_, size_t len, void *buf2, size_t len2, off_t off)
{
	switch(fn) {
	case MSREAD: {
		Server *s = (Server *) c;
		uint8_t *buf = buf_;
		size_t msgpos = 0;
		if(s->read == 0) {
			condsignal(&s->ready);
			condwait(&s->hasmsg);
		}
		if(s->read == msgpos && len >= 4) {
			PBIT32(buf, s->msg->fn);
			buf += 4, len -= 4;
			s->read += 4;
		}
		msgpos += 4;
		if(s->read == msgpos && len >= 4) {
			PBIT32(buf, s->msg->fid);
			buf += 4, len -= 4;
			s->read += 4;
		}
		msgpos += 4;
		if(s->read == msgpos && len >= 1) {
			PBIT32(buf, s->msg->submsg);
			buf += 4, len -= 4;
			s->read += 4;
		}
		msgpos += 4;
		if(s->msg->fn & MWANTSPTR) {
			if(s->read == msgpos && len >= 4) { 
				PBIT32(buf, s->msg->len);
				buf += 4, len -= 4;
				s->read += 4;
			}
			msgpos += 4;
			if(s->read == msgpos && len >= PTRLEN / 8) {
				s->msg->buf = putusrptr(s->msg->pl);
				PBITPTR(buf, s->msg->buf);
				buf += PTRLEN / 8, len -= PTRLEN / 8;
				s->read += PTRLEN / 8;
			}
			msgpos += PTRLEN / 8;
		}
		if(s->msg->fn & MWANTSPTR2) {
			if(s->read == msgpos && len >= 4) {
				PBIT32(buf, s->msg->len2);
				buf += 4, len -= 4;
				s->read += 4;
			}
			msgpos += 4;
			if(s->read == msgpos && len >= PTRLEN / 8) {
				s->msg->buf2 = putusrptr(s->msg->pl2);
				PBITPTR(buf, s->msg->buf2);
				buf += PTRLEN / 8, len -= PTRLEN / 8;
				s->read += PTRLEN / 8;
			}
			msgpos += PTRLEN / 8;
		}
		if(s->msg->fn & MWANTSOFF) {
			if(s->read == msgpos && len >= 8) {
				PBIT64(buf, s->msg->off);
				buf += 8, len -= 8;
				s->read += 8;
			}
			msgpos += 8;
		}
		if(s->read == msgpos) {
			s->read = 0;
			s->msg->next = s->msgs;
			s->msgs = s->msg;
		}
		return s->read ? s->read : msgpos;
	}
	case MSWRITE: {
		Server *s = (Server *) c;
		Msg **msg, *m;
		const uint8_t *buf = buf_;
		int fid;
		if(len != 12)
			return -EINVAL;
		fid = GBIT32(buf);
		for(msg = &s->msgs; *msg && (*msg)->fid != fid; msg = &(*msg)->next) {}
		if(!*msg)
			return -EINVAL;
		m = *msg;
		*msg = m->next;
		m->ret = GBIT64(buf + 4);
		if(m->fn & MWANTSPTR)
			freeusrptr(m->pl, m->buf);
		if(m->fn & MWANTSPTR2)
			freeusrptr(m->pl2, m->buf2);
		condsignal(&m->done);
		return len;
	}
	case MSTAT: {
		Dir d = { { 0x44, 0, 0 }, 0x44000180, 0, 0, 0, "", "", "", "" };
		return convD2M(&d, buf_, len);
	}
	case MOPEN: {
		if(submsg & (O_EXCL | O_TRUNC))
			return -1;
		return 0;
	}
	default:
		return -EINVAL;
	}
	(void) buf2, (void) len2, (void) off;
}

static Dev sops = {
	sdel,
	sdup,
	sfn
};

static long long cfn(Conn *, int, int, void *, size_t, void *, size_t, off_t);

static void cdel(Conn *c_)
{
	Client *c = (Client *) c_;
	cfn(c_, MDEL, 0, 0, 0, 0, 0, 0);
	unref(c->s);
	free(c);
}
static Conn *cdup(Conn *c_)
{
	Client *c = (Client *) c_;
	Client *newc = calloc(1, sizeof(*newc));
	conninit((Conn *)newc, c_->name->s, c->c.qid, &cops, c->s);
	newc->s = c->s;
	ref(newc->s);
	newc->fid = cfn(c_, MDUP, 0, 0, 0, 0, 0, 0);
	/* TODO handle errors */
	return (Conn *) newc;
}

static long long cfn(Conn *c_, int mesg, int sub, void *buf, size_t len, void *buf2, size_t len2, off_t off)
{
	Client *c = (Client *) c_;
	c->msg.fid = c->fid;
	c->msg.fn = mesg;
	c->msg.submsg = sub;
	c->msg.off = mesg & MWANTSOFF ? off : 0;
	if(mesg & MWANTSPTR) {
		c->msg.pl = getusrptr(buf, len);
		c->msg.len = len;
	} else {
		c->msg.pl = c->msg.buf = 0;
		c->msg.len = 0;
	}
	if(mesg & MWANTSPTR2) {
		c->msg.pl2 = getusrptr(buf2, len2);
		c->msg.len2 = len2;
	} else {
		c->msg.pl2 = c->msg.buf2 = 0;
		c->msg.len2 = 0;
	}
	sendmsg(c->s, &c->msg);
	if(mesg & MWANTSPTR)
		unref(c->msg.pl);
	if(mesg & MWANTSPTR2)
		unref(c->msg.pl2);
	return c->msg.ret;
}

static Dev cops = {
	cdel,
	cdup,
	cfn
};

