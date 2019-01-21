#define _POSIX_SOURCE
#include <stdio.h>
#include "stream.h"

int vsnprintf(char *__restrict s, int n, char const *__restrict fmt, va_list pa)
{
	FILE f = {
		.wbuf = s,
		.wbuflen = n,
		.wstart = s,
		.wpos = s
	};
#error	TODO count return value
}

