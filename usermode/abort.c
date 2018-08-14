#include "syscall.h"

void abort(void)
{
	_exits("abort");
}

