#include <sys/func.h>
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t len)
{
	return func(fd, MSREAD, 0, buf, len, 0);
}

