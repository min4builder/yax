#define __YAX__
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <codas/bit.h>
#include <codas/vector.h>
#include <yaxfs/file.h>

typedef Vec(File *) Directory;

static void freedir(File *f)
{
	Directory *dir = f->aux;
	vecdrop(dir);
	free(dir);
}

static void filedrop(void *d)
{
	filedel((File *) d);
}

File *dirnew(struct stat st, char const *name)
{
	Directory *dir = malloc(sizeof *dir);
	*dir = vecnew(File *, filedrop);
	return filenew(st, name, dir, freedir);
}

void diraddfile(File *d, File *f)
{
	Directory *dir = d->aux;
	vecpush(dir, f);
	f->lncnt++;
}

void dirdelfile(File *d, File *f)
{
	Directory *dir = d->aux;
	size_t i;
	for(i = 0; i < veclen(dir); i++) {
		if(vecget(dir, i) == f) {
			vecremswap(dir, i);
			break;
		}
	}
}

File *const *diriter(File *d)
{
	Directory *dir = d->aux;
	return veciter(dir);
}

File *const *diriternext(File *d, File *const *f)
{
	Directory *dir = d->aux;
	return veciternext(dir, f);
}

ssize_t dirreadent(File *f, char *buf, size_t len)
{
	size_t dlen = 2 + YAXstat2msg(&f->st, 0, 0);
	size_t namelen = strlen(f->name);
	dlen += 2 + namelen;
	if(len < dlen)
		return 0;
	PBIT16(buf, dlen);
	buf += 2;
	buf += YAXstat2msg(&f->st, buf, len - 2);
	PBIT16(buf, namelen);
	buf += 2;
	memcpy(buf, f->name, namelen);
	return dlen;
}

ssize_t dirreadents(File *d, File *const **const f, char *buf, size_t len)
{
	size_t rlen = 0;
	ssize_t rret;
	if(!*f)
		*f = diriter(d);
	else
		*f = diriternext(d, *f);
	while(rlen < len) {
		if(!*f)
			return rlen;
		rret = dirreadent(**f, buf + rlen, len - rlen);
		if(rret < 0)
			return rret;
		rlen += rret;
		*f = diriternext(d, *f);
	}
	return rlen;
}

File *dirwalk(File *f, char const *name, size_t len)
{
	File *const *c;
	for(c = diriter(f); c; c = diriternext(f, c)) {
		if(strncmp((*c)->name, name, len) == 0)
			return *c;
	}
	return 0;
}

File *filenew(struct stat st, char const *name, void *aux, void (*freeaux)(File *))
{
	File *f = malloc(sizeof *f);
	f->lncnt = 0;
	f->name = name;
	f->st = st;
	f->aux = aux;
	f->freeaux = freeaux;
	return f;
}

ssize_t filestat(File *, void *, size_t);
ssize_t filewstat(File *, void const *, size_t);

void filedel(File *f)
{
	f->lncnt--;
	if(f->lncnt <= 0) {
		if(f->freeaux)
			f->freeaux(f);
		free(f);
	}
}

