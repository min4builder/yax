#define __YAX__
#include <stdio.h>
#include <unistd.h>
#include <yax/func.h>
#include <yax/mount.h>

extern char initrd;
extern size_t initrdlen;

extern char **environ;

void tarfsserve(int, char *, size_t);

int main(int argc, char **argv)
{
	int fd, mnt;

	fd = __getprintk();
	if(fd != 1) {
		dup2(fd, 1);
		close(fd);
	}

	printf("Hello, World!\n");
	fflush(stdout);

	fd = mkmnt(&mnt);
	mount("/", fd, MAFTER);
	close(fd);

	if(rfork(RFPROC|RFFDG)) {
		execve("/bin/init", argv, environ);
		exits("exec");
	} else
		tarfsserve(mnt, &initrd, initrdlen);

	(void) argc;
	return 0;
}

