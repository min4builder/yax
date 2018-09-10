#define __YAX__
#include <sys/types.h>
#include <unistd.h>

pid_t fork(void)
{
	return rfork(RFPROC | RFFDG);
}

