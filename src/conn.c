#define NDEBUG
#include <sys/types.h>
#include <yax/openflags.h>
#include "conn.h"
#include "libk.h"
#include "mem/malloc.h"
#include "multitask.h"
#include "pipe.h"
#include "printk.h"
#include "mem/ref.h"

static void cfree(const RefCounted *rc)
{
	Conn *c = (Conn *) rc;
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
}

int connopen(Conn *c, int fl, int mode)
{
	int ret;
	printk("{Open ");
	iprintk(fl);
	if(fl & O_CREAT) {
		cprintk(' ');
		iprintk(mode);
	}
	printk(" = ");
	ret = c->dev->open(c, fl, mode);
	iprintk(ret);
	cprintk('}');
	return ret;
}
Conn *conndup(Conn *c, const char *name)
{
	printk("{Dup}");
	return c->dev->dup(c, name);
}
int connwalk(Conn *c, const char *elem)
{
	int i;
	printk("{Walk ");
	for(i = 0; elem[i] && elem[i] != '/'; i++)
		cprintk(elem[i]);
	printk(" = ");
	i = c->dev->walk(c, elem);
	iprintk(i);
	cprintk('}');
	return i;
}

ssize_t connread(Conn *c, void *buf, size_t len)
{
	ssize_t ret;
	printk("{Read ");
	iprintk(len);
	printk(" = ");
	ret = c->dev->read(c, buf, len);
	if(ret > 0) {
		cprintk('"');
		nprintk(ret, buf);
		printk("\" ");
	}
	iprintk(ret);
	cprintk('}');
	return ret;
}
ssize_t connwrite(Conn *c, const void *buf, size_t len)
{
	ssize_t ret;
	printk("{Write ");
	iprintk(len);
	printk(" \"");
	nprintk(len, buf);
	printk("\" = ");
	ret = c->dev->write(c, buf, len);
	iprintk(ret);
	cprintk('}');
	return ret;
}

off_t connseek(Conn *c, off_t off, int whence)
{
	off_t ret;
	printk("{Seek ");
	iprintk(off);
	cprintk(' ');
	iprintk(whence);
	printk(" = ");
	ret = c->dev->seek(c, off, whence);
	iprintk(ret);
	cprintk('}');
	return ret;
}

ssize_t connpread(Conn *c, void *buf, size_t len, off_t off)
{
	ssize_t ret;
	printk("{Pread ");
	iprintk(len);
	cprintk(' ');
	iprintk(off);
	printk(" = ");
	ret = c->dev->pread(c, buf, len, off);
	if(ret > 0) {
		cprintk('"');
		nprintk(ret, buf);
		printk("\" ");
	}
	iprintk(ret);
	cprintk('}');
	return ret;
}
ssize_t connpwrite(Conn *c, const void *buf, size_t len, off_t off)
{
	ssize_t ret;
	printk("{Pwrite ");
	iprintk(len);
	cprintk(' ');
	iprintk(off);
	printk(" \"");
	nprintk(len, buf);
	printk("\" = ");
	ret = c->dev->pwrite(c, buf, len, off);
	iprintk(ret);
	cprintk('}');
	return ret;
}

ssize_t connstat(Conn *c, void *buf, size_t len)
{
	return c->dev->stat(c, buf, len);
}
ssize_t connwstat(Conn *c, const void *buf, size_t len)
{
	return c->dev->wstat(c, buf, len);
}

