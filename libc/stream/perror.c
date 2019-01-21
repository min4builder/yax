#include <errno.h>
#include <stdio.h>
#include <string.h>

void perror(char const *s)
{
	fprintf(stderr, "%s: %s\n", s, strerror(errno));
}

