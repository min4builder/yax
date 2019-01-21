#define _POSIX_SOURCE
#include <stdio.h>

int printf(char const *fmt, ...)
{
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return ret;
}

