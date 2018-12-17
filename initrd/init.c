#define __YAX__
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

static pid_t spawn(const char *name, char **argv)
{
	pid_t p;
	if((p = fork()) > 0)
		return p;
	else if(p == 0)
		execve(name, argv, environ);
	exits("exec");
}

int main(int argc, char **argv)
{
	int fd = -1;
	ssize_t res;
	char buf[128];
	char *a[] = { "/bin/cons", 0 };
	spawn("/bin/cons", a);
	a[0] = "/bin/disk";
	spawn("/bin/disk", a);
	chdir("/dev");
	while(fd < 0)
		fd = open("cons", O_RDWR);
	while((res = read(fd, buf, sizeof(buf))) > 0)
		write(fd, buf, res);
	(void) argc, (void) argv;
	return res;
}

