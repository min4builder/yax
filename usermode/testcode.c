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

static int printfd;

void print(char *s)
{
	write(printfd, s, strlen(s));
}

void printi(unsigned int i)
{
	char is[16], *isp = is+16;
	do {
		*--isp = (i & 0xF) + ((i & 0xF) < 10 ? '0' : 'A' - 10);
		i >>= 4;
	} while(i > 0 && isp > is);
	write(printfd, isp, 16 - (isp - is));
}

extern char initrd;
extern size_t initrdlen;

int main(int argc, char **argv)
{
	int fd, mnt;

	fd = mkmnt(&mnt);
	__mountfd("/", fd, MAFTER);
	close(fd);

	printfd = __getprintk();

	if(rfork(RFPROC|RFFDG)) {
		exec("/init", "'argv'", "'envp=0'");
		exits("exec");
	} else
		tarfsserve(mnt, &initrd, initrdlen);

	exits(0);
}

