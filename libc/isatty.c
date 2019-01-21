#define _POSIX_SOURCE
#include <errno.h>
#include <unistd.h>

int isatty(int fd)
{
	/* TODO: ttys are not supported yet */
	errno = ENOTTY;
	(void) fd;
	return 0;
}

