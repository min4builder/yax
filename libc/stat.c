#define __YAX__
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <codas/bit.h>

size_t YAXstat2msg(struct stat const *__restrict st, char *__restrict buf, size_t len)
{
	size_t dlen = STATMSGSIZ;
	if(!buf || len < dlen)
		return dlen;
	PBIT64(buf, st->st_ino);
	buf += 8;
	PBIT32(buf, st->st_mode);
	buf += 4;
	PBIT64(buf, st->st_size);
	buf += 8;
	PBIT64(buf, st->st_atim.tv_sec);
	buf += 8;
	PBIT64(buf, st->st_mtim.tv_sec);
	buf += 8;
	PBIT32(buf, st->st_uid);
	buf += 4;
	PBIT32(buf, st->st_gid);
	buf += 4;
	return dlen;
}

void YAXmsg2stat(char const *__restrict buf, struct stat *__restrict st)
{
	st->st_dev = 0;
	st->st_rdev = 0;

	st->st_ino = GBIT64(buf);
	buf += 8;
	st->st_mode = GBIT32(buf);
	buf += 4;
	st->st_size = GBIT64(buf);
	buf += 8;

	st->st_atim.tv_sec = GBIT64(buf);
	st->st_atim.tv_nsec = 0;
	buf += 8;

	st->st_mtim.tv_sec = GBIT64(buf);
	st->st_mtim.tv_nsec = 0;
	buf += 8;

	st->st_ctim = st->st_mtim;

	st->st_uid = GBIT32(buf);
	buf += 4;
	st->st_gid = GBIT32(buf);
	buf += 4;
}

