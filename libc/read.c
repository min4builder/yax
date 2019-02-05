#define __YAX__
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <yax/func.h>

ssize_t read(int fd, void *buf, size_t len)
{
	ssize_t ret = func(fd, MSREAD, 0, buf, len, 0, 0, 0);
	if(ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

