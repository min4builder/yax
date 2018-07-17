/* TODO FIXME */
#include <yax/mapflags.h>
#include "syscall.h"
void *malloc(size_t len)
{
	return mmap(0, len, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, 0, 0);
}

