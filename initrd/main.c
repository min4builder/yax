#define _YAX_
#include <stdint.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <unistd.h>
#include <yax/errorcodes.h>
#include <yax/mapflags.h>
#include <yax/mountflags.h>
#include <yax/openflags.h>
#include <yax/rfflags.h>
#include <yax/stat.h>

extern char initrd;
extern size_t initrdlen;

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

	return 0;
}

