#include <stdlib.h>
#include <string.h>

char *strdup(char const *s)
{
	size_t len = strlen(s);
	char *ns = malloc(len);
	if(!ns)
		return 0;
	memcpy(ns, s, len);
	return ns;
}

