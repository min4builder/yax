#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <codas/bit.h>
#include <yax/stat.h>

size_t convD2M(Dir *d, char *buf, size_t len)
{
	size_t dlen = 49;
	size_t namelen = strlen(d->name);
	size_t uidlen = strlen(d->uid);
	size_t gidlen = strlen(d->gid);
	size_t muidlen = strlen(d->muid);
	dlen += namelen + uidlen + gidlen + muidlen;
	if(!buf)
		return dlen;
	if(len < 2)
		return 0;
	PBIT16(buf, dlen);
	buf += 2;
	if(len < dlen)
		return 2;
	PBIT16(buf, 0);
	buf += 2;
	PBIT32(buf, 0);
	buf += 4;
	PBIT8(buf, d->qid.type);
	buf += 1;
	PBIT32(buf, d->qid.vers);
	buf += 4;
	PBIT64(buf, d->qid.path);
	buf += 8;
	PBIT32(buf, d->mode);
	buf += 4;
	PBIT32(buf, d->atime);
	buf += 4;
	PBIT32(buf, d->mtime);
	buf += 4;
	PBIT64(buf, d->length);
	buf += 8;
	PBIT16(buf, namelen);
	buf += 2;
	memcpy(buf, d->name, namelen);
	buf += namelen;
	PBIT16(buf, uidlen);
	buf += 2;
	memcpy(buf, d->uid, uidlen);
	buf += uidlen;
	PBIT16(buf, gidlen);
	buf += 2;
	memcpy(buf, d->gid, gidlen);
	buf += gidlen;
	PBIT16(buf, muidlen);
	buf += 2;
	memcpy(buf, d->muid, muidlen);
	buf += muidlen;
	return dlen;
}
/* TODO */
size_t convM2D(char *, size_t, Dir *, char *);

