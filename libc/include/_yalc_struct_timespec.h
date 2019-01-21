#include <_yalc.h>

#ifndef __TIMESPEC_H
#define __TIMESPEC_H

_YALCDEFTYPE(time_t);

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

#endif

