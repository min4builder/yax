#define _POSIX_SOURCE
#include <stdio.h>
#include <string.h>
#include "stream.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

size_t fread(void *__restrict ptr, size_t size, size_t nitems, FILE *__restrict stream)
{
	size_t len = size * nitems;
	size_t readlen = MIN((size_t) (stream->rend - stream->rpos), len);
	size_t pad = readlen % size;
	readlen -= pad;
	memcpy(ptr, stream->rpos, readlen);
	stream->rpos += readlen;
	if(readlen < len) {
		ssize_t nreadlen;
		if(!stream->read) {
			stream->flags |= FILERR;
			return readlen / size;
		}
		if((nreadlen = stream->read(stream->fd, ptr + readlen + pad, len - readlen)) <= 0) {
			if(nreadlen == 0)
				stream->flags |= FILEOF;
			else
				stream->flags |= FILERR;
			return readlen / size;
		}
		if(nreadlen / size >= 1) {
			memcpy(ptr + readlen, stream->rpos, pad);
			stream->rpos += pad;
		}
		readlen += nreadlen;
		if((nreadlen = stream->read(stream->fd, stream->rbuf, stream->rbuflen)) <= 0) {
			if(nreadlen == 0)
				stream->flags |= FILEOF;
			else
				stream->flags |= FILERR;
			return readlen / size;
		}
		stream->rpos = stream->rbuf;
		stream->rend = stream->rbuf + nreadlen;
	}
	return readlen / size;
}

