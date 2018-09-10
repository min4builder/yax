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
	exit(1);
}

int main(int argc, char **argv)
{
	int fd = -1;
	ssize_t res;
	char buf[128];
	char *a[] = { "/bin/cons", 0 };
	spawn("/bin/cons", a);
	while(fd < 0)
		fd = open("/dev/cons", O_RDWR);
	while((res = read(fd, buf, sizeof(buf))) > 0)
		write(fd, buf, res);
	return res;
}

