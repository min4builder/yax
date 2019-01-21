#include <stdio.h>

int snprintf(char *__restrict s, int n, char const *__restrict fmt, ...)
{
	va_list pa;
	int ret;
	va_start(pa, fmt);
	ret = vsnprintf(s, n, fmt, pa);
	va_end(pa);
	return ret;
}

