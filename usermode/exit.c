#include <stdint.h>
#include "syscall.h"

void _fini(void);

void exits(const char *s)
{
	_fini();
	_exits(s);
}

