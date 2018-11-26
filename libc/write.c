#include <unistd.h>
#include <yax/func.h>

ssize_t write(int fd, const void *buf, size_t len)
{
	return func(fd, MSWRITE, 0, (void *) buf, len, 0, 0, 0);
}

