#include <string.h>

int memcmp(const void *a_, const void *b_, size_t n)
{
	const char *a = a_, *b = b_;
	if(n) do {
		if(*a > *b)
			return 1;
		if(*a < *b)
			return -1;
		a++, b++;
		n--;
	} while(n && *a);
	return 0;
}

