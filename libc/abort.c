#define __YAX__
#include <unistd.h>

_Noreturn void abort(void)
{
	_exits("abort");
}

