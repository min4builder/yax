#include <string.h>
#include "syscall.h"

void _libc_main(char *argv, char *envp)
{
	int fd = __getprintk();
	write(fd, "argv: ", 6);
	write(fd, argv, strlen(argv));
	write(fd, "\n", 1);
	write(fd, "envp: ", 6);
	write(fd, envp, strlen(envp));
	write(fd, "\n", 1);
	exits("done");
}

