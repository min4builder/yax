#include <string.h>

char *strstr(char const *s1, char const *s2)
{
	size_t len = strlen(s2);

	while(*s1) {
		if(strncmp(s1, s2, len) == 0)
			return (char *) s1;
		s1++;
	}

	return 0;
}

