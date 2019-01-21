#define __YAX__
#include <sys/types.h>
#include <unistd.h>
#include <yax/func.h>

ssize_t read(int fd, void *buf, size_t len)
{
	return func(fd, MSREAD, 0, buf, len, 0, 0, 0);
}

