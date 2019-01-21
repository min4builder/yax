#define _POSIX_SOURCE
#include <stdio.h>
#include <unistd.h>

int fputc(int c, FILE *stream)
{
	if(fwrite(&c, 1, 1, stream) < 1)
		return EOF;
	return c;
}

