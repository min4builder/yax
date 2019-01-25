#include <time.h>

time_t time(time_t *p)
{
	time_t curtime = 0;
	/* TODO */
	if(p)
		*p = curtime;
	return curtime;
}

