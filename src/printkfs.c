#ifdef NDEBUG
#undef NDEBUG
#endif
#define __YAX__
#include <sys/types.h>
#include <yax/openflags.h>
#include "conn.h"
#include "mem/malloc.h"
#include "printk.h"
#include "printkfs.h"
#include "stat.h"

static Dev ops;

Conn *printkfsnew(void)
{
	Qid qid = { 0x44, 0, 0 };
	Conn *c = calloc(1, sizeof(*c));
	conninit(c, "<internal kernel printk>", qid, &ops, 0);
	return c;
}

static void del(Conn *c)
{
	free(c);
}
static Conn *dup(Conn *c)
{
	ref(c);
	return c;
}

static long long fn(Conn *c, int fn, int submsg, void *buf, size_t len, off_t off)
{
	switch(fn) {
	case MSWRITE: {
		nprintk(len, (const char *) buf);
		return len;
	}
	case MSTAT: {
		Dir d = { { 0x44, 0, 0 }, 0x44000100, 0, 0, 0, "", "", "", "" };
		return convD2M(&d, buf, len);
	}
	case MOPEN: {
		if(submsg & (O_EXCL | O_TRUNC))
			return -1;
		return 0;
	}
	default:
		return -EINVAL;
	}
	(void) c, (void) off;
}

static Dev ops = {
	MIMPL(MSWRITE) | MIMPL(MSTAT) | MIMPL(MOPEN),
	del,
	dup,
	fn
};

