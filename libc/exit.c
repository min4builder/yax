#define __YAX__
#include <stdlib.h>
#include <unistd.h>

_Noreturn void exit(int n)
{
	char s[32];
	itoa(n & 0xff, s, sizeof(s));
	exits(s);
}

