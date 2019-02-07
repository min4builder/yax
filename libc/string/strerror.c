#include <errno.h>
#include <string.h>

static char const *errtab[MAXERR+1] = {
	[ENOSYS] = "Not supported",
	[EACCES] = "Forbidden",
	[EAGAIN] = "Try again",
	[EBADF] = "Bad file descriptor",
	[EEXIST] = "File exists",
	[EINVAL] = "Invalid argument",
	[EISDIR] = "Is a directory",
	[EMFILE] = "File descriptor too big",
	[ENODEV] = "No such device",
	[ENOENT] = "Does not exist",
	[ENOEXEC] = "Not executable",
	[ENOMEM] = "Not enough memory",
	[ENOTDIR] = "Not a directory",
	[ENOTSUP] = "Not supported",
	[ENOTTY] = "Not the device you are looking for",
	[ENXIO] = "No such device or address",
	[EOVERFLOW] = "Overflow",
	[EPIPE] = "Broken pipe",
	[ERANGE] = "Result too large",
	[ESRCH] = "No such process",
	[EIO] = "Input/Output error",
	[ESPIPE] = "Cannot seek",
	[MAXERR] = "(invalid error, the programmer screwed up)"
};

char *strerror(int k)
{
	if(k < 0 || k > MAXERR || !errtab[k])
		return (char *) errtab[MAXERR];
	return (char *) errtab[k];
}

