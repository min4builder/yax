#include <string.h>

int strncmp(const char *a, const char *b, size_t n)
{
	if(n) do {
		if(*a > *b)
			return 1;
		if(*a < *b)
			return -1;
		n--;
	} while(n && *a);
	return 0;
}

