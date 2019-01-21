#include <stdio.h>

int puts(char const *s)
{
	if(fputs(s, stdout) < 0 || putc('\n', stdout) < 0)
		return EOF;
	return 0;
}

