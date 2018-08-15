#include <unistd.h>

void abort(void)
{
	_exits("abort");
}

