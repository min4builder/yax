#define __YAX__
#include <codas/bit.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct dirent *readdir(DIR *d)
{
	char blen[2];
	ssize_t len;
	char buf_[256];
	char *buf;
	struct stat st;
	if(d->cur)
		free(d->cur);
	d->cur = 0;
	if(read(d->fd, blen, 2) < 2)
		return 0;
	len = GBIT16(blen);
	if(len <= 256)
		buf = buf_;
	else {
		buf = malloc(len);
		if(!buf)
			return 0;
	}
	if(read(d->fd, buf, len) < len)
		goto error;
	YAXmsg2stat(buf + 2, &st);
	len = GBIT16(buf + 2 + STATMSGSIZ);
	d->cur = malloc(sizeof(*d->cur) + len);
	if(!d->cur)
		goto error;
	d->cur->d_ino = st.st_ino;
	memcpy(d->cur->d_name, buf + 2 + STATMSGSIZ + 2, len);
	if(buf != buf_)
		free(buf);
	return d->cur;
error:
	if(buf != buf_)
		free(buf);
	return 0;
}

