#include <string.h>

void *memcpy(void *a, const void *b, size_t n)
{
	return memmove(a, b, n);
}

void *memmove(void *a_, const void *b_, size_t n)
{
	char *a = a_;
	const char *b = b_;
	if(a < b) {
		for(; n; n--, a++, b++)
			*a = *b;
	} else {
		for(a += n-1, b += n-1; n; n--, a--, b--)
			*a = *b;
	}
	return a_;
}

