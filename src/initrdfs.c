#include "conn.h"

typedef struct {
	Conn c;
	void *mod;
	size_t len;
} Initrd;

static Dev ops;

Conn *initrdnew(void *mod, size_t len)
{
	Qid qid = { 0, 0, 0 };
	Initrd *i = malloc(sizeof(Initrd));
	i->mod = mod;
	i->len = len;
	conninit((Conn *) i, "<initrd>", qid, &ops, i);
	return (Conn *) i;
}

static void del(Conn *)
{
	free(c);
}

static Dev ops = {
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

