#define _POSIX_SOURCE
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

int closedir(DIR *d)
{
	close(d->fd);
	if(d->cur)
		free(d->cur);
	free(d);
	return 0;
}

