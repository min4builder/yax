#include <unistd.h>

void _fini(void);

void exit(int n)
{
	_fini();
	_exit(n);
}

