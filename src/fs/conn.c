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
}

