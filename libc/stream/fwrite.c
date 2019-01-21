#include <stdio.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

size_t fwrite(void const *__restrict buf, size_t size, size_t nmemb, FILE *__restrict stream)
{
	size_t len = size * nmemb;
	size_t accwlen = 0;
	while(len) {
		size_t wlen = MIN(stream->wbuflen - (stream->wpos - stream->wstart), len);
		size_t pad = wlen % size;
		wlen -= pad;
		accwlen += wlen;
		memcpy(stream->wpos, buf, wlen);
		stream->wpos += wlen;
		if(wlen < len) {
			if(fflush(stream) < 0)
				return accwlen / size;
		}
		len -= wlen;
	}
	return accwlen / size;
}

