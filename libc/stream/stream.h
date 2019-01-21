#ifndef STREAM_H_
#define STREAM_H_

enum {
	FILERR = 1,
	FILEOF = 2,
	FILWALLOC = 4,
	FILRALLOC = 8,
	FILALLOC = 16,
	FILNONE = 0
};

struct _YALC_FILE {
	int (*close)(int);
	ssize_t (*read)(int, void *, size_t);
	ssize_t (*write)(int, void const *, size_t);
	off_t (*seek)(int, off_t, int);
	char *rbuf, *wbuf;
	size_t rbuflen, wbuflen;
	char *wstart, *wpos;
	char *rpos, *rend;
	int fd;
	int flags;
};

#endif /* STREAM_H_ */

