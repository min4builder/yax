#ifndef YAXFS_SERVE_H_
#define YAXFS_SERVE_H_

#include <sys/types.h>
#include <yax/fn.h>

typedef struct {
	int fn;
	int submsg;
	int fid;
	void *buf;
	size_t len;
	void *buf2;
	size_t len2;
	off_t off;
	long long ret;
} Req;

Req recv(int fd);
void answer(Req m, int fd);

#endif /* YAXFS_SERVE_H_ */

