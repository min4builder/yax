#define _POSIX_SOURCE
#include <dirent.h>
#include <fcntl.h>

DIR *opendir(char const *path)
{
	int fd = open(path, O_RDONLY);
	if(fd < 0)
		return 0;
	return fdopendir(fd);
}

