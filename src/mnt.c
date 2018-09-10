#include <string.h>
#include <yax/bit.h>
#include <yax/openflags.h>
#include "conn.h"
#include "mem/malloc.h"
#include "mem/usrboundary.h"
#include "mnt.h"
#include "multitask.h"
#include "stat.h"

typedef struct Msg Msg;
struct Msg {
	Msg *next;
	Sem done;
	enum { MSGDEL, MSGDUP, MSGPREAD, MSGREAD, MSGPWRITE, MSGWRITE, MSGSEEK, MSGSTAT, MSGWSTAT, MSGWALK, MSGOPEN } fn;
	int fid;
	union {
		struct {
			PgList *pl;
			void *buf;
			size_t len;
			off_t off;
		} rw;
		struct {
			off_t off;
			int whence;
		} seek;
		struct {
			PgList *pl;
			void *buf;
			size_t len;
		} stat;
		struct {
			int fl;
			int mode;
		} open;
		struct {
			char *path;
		} walk;
	} u;
	union {
		int iret;
		ssize_t sret;
		off_t oret;
		Qid qret;
	} r;
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

Conn *mntnew(Conn **master)
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
	*master = (Conn *) s;
	qid.type = 0x80;
	conninit((Conn *)root, "", qid, &cops, s);
	root->s = s;
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

static ssize_t sread(Conn *c, void *buf_, size_t len)
{
	/* TODO handle partial messages */
	Server *s = (Server *) c;
	uint8_t *buf = buf_;
	condsignal(&s->ready);
	condwait(&s->hasmsg);
	if(len < 1)
		return 0;
	if(s->read == 0 && len >= 1) {
		PBIT8(buf, s->msg->fn);
		buf++, len--;
		s->read++;
	}
	if(s->read == 1 && len >= 4) {
		PBIT32(buf, s->msg->fid);
		buf += 4, len -= 4;
		s->read += 4;
	}
	switch(s->msg->fn) {
	case MSGDEL:
		if(s->read == 5) {
			s->read = 0;
			s->msg->next = s->msgs;
			s->msgs = s->msg;
			return 5;
		}
		return s->read;
	case MSGDUP:
		if(s->read == 5) {
			s->read = 0;
			s->msg->next = s->msgs;
			s->msgs = s->msg;
			return 5;
		}
		return s->read;
	case MSGPREAD:
	case MSGPWRITE: {
		int p = 1;
		if(0) {
	case MSGREAD:
	case MSGWRITE:
			p = 0;
		}
		s->msg->u.rw.buf = putusrptr(s->msg->u.rw.pl);
		if(s->read == 5 && len >= 4) { 
			PBIT32(buf, s->msg->u.rw.len);
			buf += 4, len -= 4;
			s->read += 4;
		}
		if(s->read == 9 && len >= PTRLEN / 8) {
			PBITPTR(buf, s->msg->u.rw.buf);
			buf += PTRLEN / 8, len -= PTRLEN / 8;
			s->read += PTRLEN / 8;
		}
		if(p && s->read == 9 + PTRLEN / 8 && len >= 8) {
			PBIT64(buf, s->msg->u.rw.off);
			buf += 8, len -= 8;
			s->read += 8;
		}
		if(s->read == (p ? 17 : 9) + PTRLEN / 8) {
			s->read = 0;
			s->msg->next = s->msgs;
			s->msgs = s->msg;
			return (p ? 17 : 9) + PTRLEN / 8;
		}
		return s->read;
	}
	case MSGSEEK:
		if(s->read == 5 && len >= 8) {
			PBIT64(buf, s->msg->u.seek.off);
			buf += 8, len -= 8;
			s->read += 8;
		}
		if(s->read == 13 && len >= 1) {
			*buf = s->msg->u.seek.whence;
			buf++, len--;
			s->read++;
		}
		if(s->read == 14) {
			s->read = 0;
			s->msg->next = s->msgs;
			s->msgs = s->msg;
			return 14;
		}
		return s->read;
	case MSGSTAT:
	case MSGWSTAT:
		if(s->read == 5 && len >= 4) {
			PBIT32(buf, s->msg->u.stat.len);
			buf += 4, len -= 4;
			s->read += 4;
		}
		if(s->read == 9 && len >= PTRLEN / 8) {
			s->msg->u.stat.buf = putusrptr(s->msg->u.stat.pl);
			PBITPTR(buf, s->msg->u.stat.buf);
			buf += PTRLEN / 8, len -= PTRLEN / 8;
			s->read += PTRLEN / 8;
		}
		if(s->read == 9 + PTRLEN / 8) {
			s->read = 0;
			s->msg->next = s->msgs;
			s->msgs = s->msg;
			return 9 + PTRLEN / 8;
		}
		return s->read;
	case MSGWALK: {
		size_t sl = strlen(s->msg->u.walk.path);
		if(s->read == 5 && len >= 4) {
			PBIT32(buf, sl);
			buf += 4, len -= 4;
			s->read += 4;
		}
		if(s->read == 9 && len >= sl) {
			memcpy(buf, s->msg->u.walk.path, sl);
			buf += sl, len -= sl;
			s->read += sl;
		}
		if(s->read == 9 + sl) {
			s->read = 0;
			s->msg->next = s->msgs;
			s->msgs = s->msg;
			return 9 + sl;
		}
		return s->read;
	}
	case MSGOPEN:
		if(s->read == 5 && len >= 4) {
			PBIT32(buf, s->msg->u.open.fl);
			buf += 4, len -= 4;
			s->read += 4;
		}
		if(s->msg->u.open.fl & OCREAT) {
			if(s->read == 9 && len >= 4) {
				PBIT32(buf, s->msg->u.open.mode);
				buf += 4, len -= 4;
				s->read += 4;
			}
			if(s->read == 13) {
				s->read = 0;
				s->msg->next = s->msgs;
				s->msgs = s->msg;
				return 13;
			}
			return s->read;
		}
		if(s->read == 9) {
			s->read = 0;
			s->msg->next = s->msgs;
			s->msgs = s->msg;
			return 9;
		}
		return s->read;
	default:
		return -1;
	}
}
static ssize_t spread(Conn *c, void *buf, size_t len, off_t off)
{
	(void) off;
	return sread(c, buf, len);
}

static ssize_t swrite(Conn *c, const void *buf_, size_t len)
{
	Server *s = (Server *) c;
	Msg **msg, *m;
	const uint8_t *buf = buf_;
	int fid;
	if(len < 4)
		return 0;
	fid = GBIT32(buf);
	for(msg = &s->msgs; *msg && (*msg)->fid != fid; msg = &(*msg)->next) {}
	if(!*msg)
		return -1;
	m = *msg;
	*msg = m->next;
	switch(m->fn) {
	case MSGDEL:
		if(len != 4)
			return 0;
		break;
	case MSGDUP:
		if(len != 8)
			return 0;
		m->r.iret = GBIT32(buf + 4);
		break;
	case MSGPREAD:
	case MSGPWRITE:
	case MSGREAD:
	case MSGWRITE:
		if(len != 8)
			return 0;
		freeusrptr(m->u.rw.pl, m->u.rw.buf);
		m->r.sret = GBIT32(buf + 4);
		break;
	case MSGSEEK:
		if(len != 12)
			return 0;
		m->r.oret = GBIT64(buf + 4);
		break;
	case MSGSTAT:
	case MSGWSTAT:
		if(len != 8)
			return 0;
		m->r.sret = GBIT32(buf + 4);
		break;
	case MSGWALK:
		if(len != 17)
			return 0;
		m->r.qret.type = GBIT8(buf + 4);
		m->r.qret.path = GBIT64(buf + 5);
		m->r.qret.vers = GBIT32(buf + 13);
		break;
	case MSGOPEN:
		if(len != 8)
			return 0;
		m->r.iret = GBIT32(buf + 4);
		break;
	default:
		return -1;
	}
	condsignal(&m->done);
	return len;
}
static ssize_t spwrite(Conn *c, const void *buf, size_t len, off_t off)
{
	(void) off;
	return swrite(c, buf, len);
}

static off_t sseek(Conn *c, off_t off, int whence)
{
	(void) c, (void) off, (void) whence;
	return -ESPIPE;
}

static ssize_t sstat(Conn *c, void *buf, size_t len)
{
	Dir d = { { 0x44, 0, 0 }, 0x44000180, 0, 0, 0, "", "", "", "" };
	(void) c;
	return convD2M(&d, buf, len);
}
static ssize_t swstat(Conn *c, const void *buf, size_t len)
{
	(void) c, (void) buf, (void) len;
	return -1;
}

static int swalk(Conn *c, const char *path)
{
	(void) c, (void) path;
	return -1;
}
static int sopen(Conn *c, int fl, int mode)
{
	if(fl & (OEXCL | OTRUNC))
		return -1;
	(void) c, (void) mode;
	return 0;
}

static Dev sops = {
	sdel,
	sdup,
	spread,
	sread,
	spwrite,
	swrite,
	sseek,
	sstat,
	swstat,
	swalk,
	sopen
};

static void cdel(Conn *c_)
{
	Client *c = (Client *) c_;
	c->msg.fn = MSGDEL;
	sendmsg(c->s, &c->msg);
	unref(c->s);
	free(c);
}
static Conn *cdup(Conn *c_)
{
	Client *c = (Client *) c_;
	Client *newc = calloc(1, sizeof(*newc));
	conninit((Conn *)newc, "", c->c.qid, &cops, c->s);
	newc->s = c->s;
	ref(newc->s);
	c->msg.fn = MSGDUP;
	c->msg.fid = c->fid;
	sendmsg(c->s, &c->msg);
	newc->fid = c->msg.r.iret;
	/* TODO handle errors */
	return (Conn *) newc;
}

static ssize_t cpread(Conn *c_, void *buf, size_t len, off_t off)
{
	Client *c = (Client *) c_;
	seminit(&c->msg.done, 0);
	c->msg.fn = MSGPREAD;
	c->msg.fid = c->fid;
	c->msg.u.rw.pl = getusrptr(buf, len);
	c->msg.u.rw.off = off;
	c->msg.u.rw.len = len;
	sendmsg(c->s, &c->msg);
	return c->msg.r.sret;
}
static ssize_t cread(Conn *c_, void *buf, size_t len)
{
	Client *c = (Client *) c_;
	seminit(&c->msg.done, 0);
	c->msg.fn = MSGREAD;
	c->msg.fid = c->fid;
	c->msg.u.rw.pl = getusrptr(buf, len);
	c->msg.u.rw.off = 0;
	c->msg.u.rw.len = len;
	sendmsg(c->s, &c->msg);
	return c->msg.r.sret;
}
static ssize_t cpwrite(Conn *c_, const void *buf, size_t len, off_t off)
{
	Client *c = (Client *) c_;
	seminit(&c->msg.done, 0);
	c->msg.fn = MSGPWRITE;
	c->msg.fid = c->fid;
	c->msg.u.rw.pl = getusrptr(buf, len);
	c->msg.u.rw.off = off;
	c->msg.u.rw.len = len;
	sendmsg(c->s, &c->msg);
	return c->msg.r.sret;
}
static ssize_t cwrite(Conn *c_, const void *buf, size_t len)
{
	Client *c = (Client *) c_;
	seminit(&c->msg.done, 0);
	c->msg.fn = MSGWRITE;
	c->msg.fid = c->fid;
	c->msg.u.rw.pl = getusrptr(buf, len);
	c->msg.u.rw.off = 0;
	c->msg.u.rw.len = len;
	sendmsg(c->s, &c->msg);
	return c->msg.r.sret;
}

static off_t cseek(Conn *c_, off_t off, int whence)
{
	Client *c = (Client *) c_;
	seminit(&c->msg.done, 0);
	c->msg.fn = MSGSEEK;
	c->msg.fid = c->fid;
	c->msg.u.seek.off = off;
	c->msg.u.seek.whence = whence;
	sendmsg(c->s, &c->msg);
	return c->msg.r.oret;
}

static ssize_t cstat(Conn *c_, void *buf, size_t len)
{
	Client *c = (Client *) c_;
	seminit(&c->msg.done, 0);
	c->msg.fn = MSGSTAT;
	c->msg.fid = c->fid;
	c->msg.u.stat.pl = getusrptr(buf, len);
	c->msg.u.stat.len = len;
	sendmsg(c->s, &c->msg);
	return c->msg.r.sret;
}
static ssize_t cwstat(Conn *c_, const void *buf, size_t len)
{
	Client *c = (Client *) c_;
	seminit(&c->msg.done, 0);
	c->msg.fn = MSGWSTAT;
	c->msg.fid = c->fid;
	c->msg.u.stat.pl = getusrptr(buf, len);
	c->msg.u.stat.len = len;
	sendmsg(c->s, &c->msg);
	return c->msg.r.sret;
}

static int cwalk(Conn *c_, const char *path)
{
	Client *c = (Client *) c_;
	const char *pw;
	seminit(&c->msg.done, 0);
	c->msg.fn = MSGWALK;
	c->msg.fid = c->fid;
	for(pw = path; *pw && *pw != '/'; pw++) {}
	c->msg.u.walk.path = malloc(pw - path + 1);
	memcpy(c->msg.u.walk.path, path, pw - path);
	c->msg.u.walk.path[pw - path] = '\0';
	sendmsg(c->s, &c->msg);
	free(c->msg.u.walk.path);
	c_->qid = c->msg.r.qret;
	return c_->qid.type == 0xFF ? (int) c_->qid.path : 0;
}
static int copen(Conn *c_, int fl, int mode)
{
	Client *c = (Client *) c_;
	seminit(&c->msg.done, 0);
	c->msg.fn = MSGOPEN;
	c->msg.fid = c->fid;
	c->msg.u.open.fl = fl;
	c->msg.u.open.mode = mode;
	sendmsg(c->s, &c->msg);
	return c->msg.r.iret;
}

static Dev cops = {
	cdel,
	cdup,
	cpread,
	cread,
	cpwrite,
	cwrite,
	cseek,
	cstat,
	cwstat,
	cwalk,
	copen
};

