#define __YAX__
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void _Yalcassert(int x, const char *func, const char *msg)
{
	if(!x) {
		char m[ERRMAX];
		strlcpy(m, "Assertion failed at ", ERRMAX);
		strlcat(m, func, ERRMAX);
		strlcat(m, ":", ERRMAX);
		strlcat(m, msg, ERRMAX);
		write(2, m, strlen(m));
		abort();
	}
}

