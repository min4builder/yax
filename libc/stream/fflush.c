#define _POSIX_SOURCE
#include <stdio.h>
#include "stream.h"

int fflush(FILE *stream)
{
	ssize_t w;
	if(!stream->write) {
		stream->flags |= FILERR;
		return EOF;
	}
	w = stream->write(stream->fd, stream->wstart, stream->wpos - stream->wstart);
	if(w < 0) {
		stream->flags |= FILERR;
		return EOF;
	}
	stream->wstart += w;
	if(stream->wstart == stream->wpos) {
		stream->wpos = stream->wstart = stream->wbuf;
		return 0;
	}
	stream->flags |= FILERR;
	return EOF;
}

