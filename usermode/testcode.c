#include <stdint.h>
#include <yax/rfflags.h>
#include "syscall.h"

void exits(const char *);

void _libc_main(char *argv, char *envp)
{
	int fds[2];

	pipe(fds);

	if(rfork(RFPROC|RFFDG)) {
		close(fds[1]);
		pwrite(fds[0], "aoeui\n", 6, 0);
	} else {
		close(fds[0]);
		char b[3];
		size_t l;
		while((l = read(fds[1], b, 3))) {
			int i;
			for(i = 0; i < l; i++)
				__cprintk(b[i]);
		}
		b[2] = '\0';
		exits(b);
	}

	exits(0);
}

