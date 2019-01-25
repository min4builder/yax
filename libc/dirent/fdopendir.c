#define _POSIX_SOURCE
#include <dirent.h>
#include <stdlib.h>

DIR *fdopendir(int fd)
{
	DIR *d = malloc(sizeof(*d));
	if(!d)
		return 0;
	d->fd = fd;
	d->cur = 0;
	return d;
}

