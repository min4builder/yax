#include <string.h>

char *strcpy(char *a, const char *b)
{
	char *c = a;
	do *c++ = *b++; while(*b);
	return a;
}

