#define _POSIX_SOURCE
#include <stdio.h>

int vprintf(char const *fmt, va_list ap)
{
	return vfprintf(stdout, fmt, ap);
}

