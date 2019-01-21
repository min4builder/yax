#define _POSIX_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int fputs(char const *s, FILE *stream)
{
	size_t len = strlen(s);
	if(fwrite(s, 1, len, stream) < len)
		return EOF;
	return 0;
}

