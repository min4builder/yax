/* TODO FIXME */
#include <yax/errorcodes.h>
#include <yax/mapflags.h>
#include "syscall.h"

void *malloc(size_t len)
{
	void *p = mmap(0, len + sizeof(len), PROT_READ | PROT_WRITE, MAP_ANONYMOUS, 0, 0);
	if(p < 0 && p > -MAXERR)
		return p;
	*((size_t *)p) = len;
	return p + sizeof(len);
}

void free(void *p)
{
	p -= sizeof(size_t);
	munmap(p, *((size_t *) p));
}

