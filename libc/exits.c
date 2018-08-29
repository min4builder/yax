#define _YAX_
#include <stdint.h>
#include <unistd.h>

void _fini(void);

void exits(const char *s)
{
	_fini();
	_exits(s);
}

