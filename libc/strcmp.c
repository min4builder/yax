#include <string.h>

int strcmp(const char *a, const char *b)
{
	do {
		if(*a > *b)
			return 1;
		if(*a < *b)
			return -1;
		a++, b++;
	} while(*a);
	return 0;
}

