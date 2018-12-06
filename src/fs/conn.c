#define NDEBUG
#define __YAX__
#include <string.h>
#include <sys/types.h>
#include <codas/bit.h>
#include <codas/ref.h>
#include <yax/openflags.h>
#include "fs/conn.h"
#include "mem/malloc.h"
#include "multitask.h"
#include "printk.h"

static void cfree(const RefCounted *rc)
{
	Conn *c = (Conn *) rc;
	printk("{Del}");
	unref(c->name);
	c->dev->del(c);
}
void conninit(Conn *c, const char *name, Qid qid, Dev *dev, void *inst)
{
	size_t namelen = strlen(name);
	char *nname;
	mkref(c, cfree);
	nname = malloc(namelen + 1);
	memcpy(nname, name, namelen + 1);
	c->name = strmk(nname);
	c->qid = qid;
	c->dev = dev;
	c->inst = inst;
}

Conn *conndup(Conn *c, Str *name)
{
	Conn *d;
	printk("{Dup}");
	d = c->dev->dup(c);
	ref(name);
	d->name = name;
	return d;
}

long long connfunc(Conn *c, int f, int sf, void *buf, size_t len, off_t off)
{
	return connfuncpp(c, f, sf, buf, len, 0, 0, off);
}

long long connfuncpp(Conn *c, int f, int sf, void *buf, size_t len, void *buf2, size_t len2, off_t off)
{
	printk("{");
	iprintk(f & MWANTSMASK);
	printk(" ");
	xprintk(sf);
	if(f & MWANTSPTR) {
		printk(" ");
		iprintk(len);
		if(!(f & MWANTSWR)) {
			printk(" ");
			nprintk(len, buf);
		}
	}
	if(f & MWANTSPTR2) {
		printk(" ");
		iprintk(len2);
		if(!(f & MWANTSWR2)) {
			printk(" ");
			nprintk(len2, buf2);
		}
	}
	if(f & MWANTSOFF) {
		printk(" ");
		iprintk(off);
	}
	printk("}");
	if(f == MWALK) {
		char walk[13];
		long long ret = c->dev->f(c, f, sf, buf, len, walk, sizeof walk, off);
		if(ret >= 0) {
			c->qid.type = walk[0];
			c->qid.vers = GBIT32(walk+1);
			c->qid.path = GBIT64(walk+5);
		}
		return ret;
	}
	return c->dev->f(c, f, sf, buf, len, buf2, len2, off);
#if 0
	/* default actions */
	switch(f) {
	case MOPEN: {
		/* ??? */
		return -EACCES;
	}
	case MWALK: {
		return -ENOTDIR;
	}
	case MSREAD: {
		if(c->impl & MIMPL(MPREAD)) {
			ssize_t ret;
			off = connfunc(c, MSEEK, 1, 0, 0, 0);
			if(off < 0) return off;
			ret = connfunc(c, MPREAD, 0, buf, len, off);
			if(ret < 0) return ret;
			off = connfunc(c, MSEEK, 1, 0, 0, len);
			if(off < 0) return off;
			return ret;
		} else {
			return -EACCES;
		}
	}
	case MSWRITE: {
		if(c->impl & MIMPL(MPWRITE)) {
			ssize_t ret;
			off = connfunc(c, MSEEK, 1, 0, 0, 0);
			if(off < 0) return off;
			ret = connfunc(c, MPWRITE, 0, buf, len, off);
			if(ret < 0) return ret;
			off = connfunc(c, MSEEK, 1, 0, 0, len);
			if(off < 0) return off;
			return ret;
		} else {
			return -EACCES;
		}
	}
	case MSEEK: {
		if(c->impl & (MIMPL(MPREAD) | MIMPL(MPWRITE))) {
			/* TODO */
			return -EIO;
		} else {
			return -EIO;
		}
	}
	case MPREAD: {
		if(c->impl & MIMPL(MSREAD)) {
			off_t ooff = connfunc(c, MSEEK, 0, 0, 0, off);
			ssize_t ret;
			if(ooff < 0) return ooff;
			ret = connfunc(c, MSREAD, 0, buf, len, 0);
			if(ret < 0) return ret;
			ooff = connfunc(c, MSEEK, 0, 0, 0, ooff);
			if(ooff < 0) return ooff;
			return ret;
		} else {
			return -EACCES;
		}
	}
	case MPWRITE: {
		if(c->impl & MIMPL(MSWRITE)) {
			off_t ooff = connfunc(c, MSEEK, 0, 0, 0, off);
			ssize_t ret;
			if(ooff < 0) return ooff;
			ret = connfunc(c, MSWRITE, 0, buf, len, 0);
			if(ret < 0) return ret;
			ooff = connfunc(c, MSEEK, 0, 0, 0, ooff);
			if(ooff < 0) return ooff;
			return ret;
		} else {
			return -EACCES;
		}
	}
	case MSTAT: {
		return -EACCES;
	}
	case MWSTAT: {
		return -EACCES;
	}
	default:
		return -EINVAL;
	}
#endif
}

