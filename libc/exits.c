#define __YAX__
#include <unistd.h>

void _fini(void);

void exits(const char *s)
{
	_fini();
	_exits(s);
}

