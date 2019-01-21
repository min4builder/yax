#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "stream.h"

int fclose(FILE *stream)
{
	fflush(stream);
	if(stream->close)
		stream->close(stream->fd);
	if(stream->flags & FILWALLOC)
		free(stream->wbuf);
	if(stream->flags & FILRALLOC)
		free(stream->rbuf);
	if(stream->flags & FILALLOC)
		free(stream);
	return 0;
}

