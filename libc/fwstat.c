#define __YAX__
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <codas/bit.h>
#include <yax/func.h>

int fwstat(int fd, struct stat const *st)
{
	char buf[256];
	size_t len = YAXstat2msg(st, buf, sizeof(buf));
	int ret = func(fd, MWSTAT, 0, buf, len, 0, 0, 0);
	if(ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

