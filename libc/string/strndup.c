#define _BSD_SOURCE
#include <stdlib.h>
#include <string.h>

char *strndup(char const *s, size_t n)
{
	char *ns = malloc(n);
	if(!ns)
		return 0;
	strlcpy(ns, s, n);
	return ns;
}

