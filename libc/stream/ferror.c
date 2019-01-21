#define _POSIX_SOURCE
#include <stdio.h>
#include "stream.h"

int ferror(FILE *stream)
{
	return !!(stream->flags & FILERR);
}

