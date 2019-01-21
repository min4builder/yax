#include <string.h>

char *strchr(char const *s, int c)
{
	/* note that c might be 0 */
	while(*s != c) {
		if(!*s) return 0;
		s++;
	}

	return (char *) s;
}

