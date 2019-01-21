#include <string.h>

char *strrchr(char const *s, int c)
{
	char const *ns = 0;

	while(*s) {
		if(*s == c)
			ns = s;
		s++;
	}

	return (char *) ns;
}

