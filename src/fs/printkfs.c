#ifdef NDEBUG
#undef NDEBUG
#endif
#define __YAX__
#include <sys/stat.h>
#include <sys/types.h>
#include <yax/errorcodes.h>
#include <yax/openflags.h>
#include "fs/conn.h"
#include "fs/printkfs.h"
#include "mem/malloc.h"
#include "printk.h"

static Dev ops;

Conn *printkfsnew(void)
{
	Conn *c = calloc(1, sizeof(*c));
	conninit(c, "<internal kernel printk>", 0, &ops, 0);
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

static long long fn(Conn *c, int fn, int submsg, void *buf, size_t len, void *buf2, size_t len2, off_t off)
{
	switch(fn) {
	case MSWRITE: {
		nprintk(len, (const char *) buf);
		return len;
	}
	case MSTAT: {
		struct stat st = { .st_ino = 0, .st_mode = S_IFIFO | 0200 };
		return YAXstat2msg(&st, buf, len);
	}
	case MOPEN: {
		if(submsg & (O_EXCL | O_TRUNC))
			return -EACCES;
		return 0;
	}
	default:
		return -ENOSYS;
	}
	(void) c, (void) buf2, (void) len2, (void) off;
}

static Dev ops = {
	del,
	dup,
	fn
};

