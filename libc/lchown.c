#define __YAX__
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int lchown(char const *path, uid_t u, gid_t g)
{
	int fd = open(path, O_PATH);
	int ret;
	if(fd < 0)
		return -1;
	ret = fchown(fd, u, g);
	close(fd);
	return ret;
}

int chown(char const *path, uid_t u, gid_t g)
{
	/* TODO implement symlinks */
	return lchown(path, u, g);
}

