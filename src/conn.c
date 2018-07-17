#include <sys/types.h>
#include <yax/openflags.h>
#include "conn.h"
#include "libk.h"
#include "malloc.h"
#include "multitask.h"
#include "pipe.h"
#include "ref.h"

static void cfree(const RefCounted *rc)
{
	Conn *c = rc;
	c->dev->del(c);
}
void conninit(Conn *c, const char *name, Qid qid, Dev *dev, void *inst)
{
	size_t namelen = strlen(name);
	mkref(c, cfree);
	c->name = malloc(namelen + 1);
	memcpy(c->name, name, namelen + 1);
	c->qid = qid;
	c->dev = dev;
	c->inst = inst;
	c->off = 0;
}

int connopen(Conn *c, int fl, int mode)
{
	return c->dev->open(c, fl, mode);
}
Conn *conndup(Conn *c, const char *name)
{
	return c->dev->dup(c, name);
}
int connwalk(Conn *c, const char *elem)
{
	return c->dev->walk(c, elem);
}

ssize_t connread(Conn *c, void *buf, size_t len)
{
	ssize_t ret = c->dev->pread(c, buf, len, c->off);
	if(ret > 0)
		c->off += ret;
	return ret;
}
ssize_t connwrite(Conn *c, const void *buf, size_t len)
{
	ssize_t ret = c->dev->pwrite(c, buf, len, c->off);
	if(ret > 0)
		c->off += ret;
	return ret;
}
off_t connseek(Conn *c, off_t off, int whence)
{
	switch(whence) {
	case 0:
		c->off = off;
		break;
	case 1:
		c->off += off;
		break;
	case 2:
		/* TODO */
		return -1;
	default:
		return -1;
	}
	return 0;
}

ssize_t connpread(Conn *c, void *buf, size_t len, off_t off)
{
	return c->dev->pread(c, buf, len, off);
}
ssize_t connpwrite(Conn *c, const void *buf, size_t len, off_t off)
{
	return c->dev->pwrite(c, buf, len, off);
}

ssize_t connstat(Conn *c, void *buf, size_t len)
{
	return c->dev->stat(c, buf, len);
}
ssize_t connwstat(Conn *c, const void *buf, size_t len)
{
	return c->dev->wstat(c, buf, len);
}

