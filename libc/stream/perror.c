#include <errno.h>
#include <stdio.h>
#include <string.h>

void perror(char const *s)
{
	if(!s || !*s)
		fprintf(stderr, "%s\n", strerror(errno));
	else
		fprintf(stderr, "%s: %s\n", s, strerror(errno));
}

