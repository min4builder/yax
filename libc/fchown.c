#define __YAX__
#include <sys/stat.h>
#include <unistd.h>

int fchown(int fd, uid_t u, gid_t g)
{
	struct stat st;
	if(fstat(fd, &st) < 0)
		return -1;
	st.st_uid = u;
	st.st_gid = g;
	if(fwstat(fd, &st) < 0)
		return -1;
	return 0;
}

