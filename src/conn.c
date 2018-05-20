#include <sys/types.h>
#include "conn.h"
#include "libk.h"
#include "malloc.h"
#include "multitask.h"
#include "pipe.h"

void conninit(Conn *c, int kind, const char *name)
{
	size_t namelen = strlen(name);
	c->name = malloc(namelen + 1);
	memcpy(c->name, name, namelen + 1);
	c->kind = kind;
	c->refcnt = 1;
}
void connref(Conn *c)
{
	locklock(&c->l);
	c->refcnt++;
	lockunlock(&c->l);
}
void connunref(Conn *c)
{
	locklock(&c->l);
	c->refcnt--;
	if(c->refcnt <= 0) {
		if(c->kind == CONNPIPE)
			pipedel((Pipe *)c);
		else if(c->kind == CONNMUX)
			/* TODO */ {}
		return;
	}
	lockunlock(&c->l);
}

ssize_t connread(Conn *c, void *buf, size_t len)
{
	if(c->kind == CONNPIPE)
		return piperead((Pipe *)c, buf, len);
	return -1;
}
ssize_t connwrite(Conn *c, const void *buf, size_t len)
{
	if(c->kind == CONNPIPE)
		return pipewrite((Pipe *)c, buf, len);
	return -1;
}
off_t connseek(Conn *c, off_t off, int whence)
{
	if(c->kind == CONNPIPE)
		return 0;
	return -1;
}

ssize_t connpread(Conn *c, void *buf, size_t len, off_t off)
{
	if(c->kind == CONNPIPE)
		return piperead((Pipe *)c, buf, len);
	(void) off;
	return -1;
}
ssize_t connpwrite(Conn *c, const void *buf, size_t len, off_t off)
{
	if(c->kind == CONNPIPE)
		return pipewrite((Pipe *)c, buf, len);
	(void) off;
	return -1;
}

ssize_t connstat(Conn *c, void *buf, size_t len)
{
	if(c->kind == CONNPIPE)
		return pipestat((Pipe *)c, buf, len);
	return -1;
}
ssize_t connwstat(Conn *c, const void *buf, size_t len)
{
	if(c->kind == CONNPIPE)
		return pipewstat((Pipe *)c, buf, len);
	return -1;
}

