#define __YAX__
#include <stdlib.h>
#include <unistd.h>

void _fini(void);

void _exit(int n)
{
	char s[32];
	itoa(n & 0xff, s, sizeof(s));
	_exits(s);
}

