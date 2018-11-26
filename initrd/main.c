#define __YAX__
#include <unistd.h>
#include <yax/func.h>
#include <yax/mount.h>

extern char initrd;
extern size_t initrdlen;

extern char **environ;

int tarfsmkmnt(int *);
void tarfsserve(int, char *, size_t);

int main(int argc, char **argv)
{
	int fd, mnt;

	fd = tarfsmkmnt(&mnt);
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

