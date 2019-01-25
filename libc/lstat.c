#define __YAX__
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int lstat(char const *__restrict path, struct stat *__restrict buf)
{
	int fd = open(path, O_PATH);
	int ret;
	if(fd < 0)
		return -1;
	ret = fstat(fd, buf);
	close(fd);
	return ret;
}

int stat(char const *__restrict path, struct stat *__restrict buf)
{
	/* TODO implement symlinks */
	return lstat(path, buf);
}

