#define __YAX__
#include <sys/mount.h>
#include <unistd.h>

extern char initrd;
extern size_t initrdlen;

extern char **environ;

void tarfsserve(int, char *, size_t);

int main(int argc, char **argv)
{
	int fd, mnt;

	fd = mkmnt(&mnt);
	__mountfd("/", fd, MAFTER);
	close(fd);

	if(rfork(RFPROC|RFFDG)) {
		execve("/bin/init", argv, environ);
		exits("exec");
	} else
		tarfsserve(mnt, &initrd, initrdlen);

	(void) argc;
	return 0;
}

