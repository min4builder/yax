#ifndef _SERVE_H
#define _SERVE_H

#include <sys/types.h>
#include <yax/fn.h>
#include <yax/stat.h>

typedef struct {
	int fn;
	int submsg;
	int fid;
	void *buf;
	size_t len;
	off_t off;
	long long ret;
} Req;

Req recv(int fd);
void answer(Req m, int fd);

#endif /* _SERVE_H */

