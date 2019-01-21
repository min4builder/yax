#include <stdio.h>

int fprintf(FILE *__restrict stream, char const *__restrict fmt, ...)
{
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vfprintf(stream, fmt, ap);
	va_end(ap);
	return ret;
}

