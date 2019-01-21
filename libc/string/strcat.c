#include <string.h>

char *strcat(char *restrict a, const char *restrict b)
{
	char *orig = a;
	while(*a) a++;
	do *a++ = *b++;
	while(*b);
	return orig;
}

