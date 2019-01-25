#define _POSIX_SOURCE
#include <time.h>

struct tm *localtime(time_t const *t)
{
	static struct tm tm;
	/* TODO */
	(void) t;
	return &tm;
}

