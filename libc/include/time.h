#include <_yalc.h>

#if !defined(_TIME_H_ANSI)
#define _TIME_H_ANSI

#ifndef NULL
#define NULL _YalcNULL
#endif

#define CLOCKS_PER_SEC 1000000

_YALCDEFTYPE(clock_t);
_YALCDEFTYPE(size_t);
_YALCDEFTYPE(time_t);

struct tm {
	int tm_sec,
	    tm_min,
	    tm_hour,
	    tm_mday,
	    tm_mon,
	    tm_year,
	    tm_wday,
	    tm_yday,
	    tm_isdst;
};

struct tm *localtime(time_t const *);
time_t time(time_t *);

#endif /* _TIME_H_ANSI */

#if !defined(_TIME_H_POSIX) && defined(_YALC_NEED_POSIX)
#define _TIME_H_POSIX

#include <_yalc_struct_timespec.h>

struct itimerspec {
	struct timespec it_interval, it_value;
};

#endif /* _TIME_H_POSIX */

