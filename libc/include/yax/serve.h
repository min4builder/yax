#ifndef YAX_SERVE_H_
#define YAX_SERVE_H_

#include <sys/types.h>
#include <yax/fn.h>

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

#endif /* YAX_SERVE_H_ */

