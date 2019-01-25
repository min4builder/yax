#define __YAX__
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <codas/bit.h>
#include <yax/func.h>

int fstat(int fd, struct stat *st)
{
	char buf_[256], *buf = buf_;
	int ret = func(fd, MSTAT, 0, buf, sizeof(buf_), 0, 0, 0);
	if(ret < 0) {
		errno = -ret;
		return -1;
	}
	return YAXmsg2stat(buf, st, ret);
}

