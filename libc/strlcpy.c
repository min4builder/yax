#include <string.h>

size_t strlcpy(char *a, const char *b, size_t n)
{
	size_t r = 0;
	while(n - 1 && *b) {
		*a++ = *b++;
		n--;
		r++;
	}
	while(*b)
		r++, b++;
	if(n == 1)
		*a++ = '\0';
	return r;
}

