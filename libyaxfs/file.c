#define __YAX__
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <codas/bit.h>
#include <yaxfs/file.h>

typedef struct {
	int nents, cap;
	File **ents;
} Directory;

static void freedir(File *f)
{
	Directory *dir = f->aux;
	int i;
	for(i = 0; i < dir->nents; i++)
		filedel(dir->ents[i]);
	free(dir->ents);
	free(dir);
}

File *dirnew(struct stat st, char const *name)
{
	Directory *dir = malloc(sizeof *dir);
	dir->nents = 0;
	dir->ents = 0;
	dir->cap = 0;
	return filenew(st, name, dir, freedir);
}

void diraddfile(File *d, File *f)
{
	Directory *dir = d->aux;
	if(++dir->nents >= dir->cap) {
		dir->cap *= 2;
		if(dir->cap == 0)
			dir->cap = 16;
		dir->ents = realloc(dir->ents, (sizeof *dir->ents) * dir->cap);
	}
	dir->ents[dir->nents-1] = f;
	f->lncnt++;
}

void dirdelfile(File *d, File *f)
{
	Directory *dir = d->aux;
	int i;
	for(i = 0; i < dir->nents; i++) {
		if(dir->ents[i] == f) {
			memmove(&dir->ents[i], &dir->ents[i+1], (dir->nents - i) * sizeof *dir->ents);
			break;
		}
	}
	if(--dir->nents < dir->cap / 2) {
		dir->cap /= 2;
		dir->ents = realloc(dir->ents, (sizeof *dir->ents) * dir->cap);
	}
}

File **dirnext(File *d, File **f)
{
	Directory *dir = d->aux;
	if(!f)
		return dir->ents;
	if(f - dir->ents > dir->nents)
		return 0;
	return f + 1;
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

ssize_t dirreadents(File *d, File ***f, char *buf, size_t len)
{
	size_t rlen = 0;
	ssize_t rret;
	while(rlen < len) {
		File **nf = dirnext(d, *f);
		if(!nf)
			return rlen;
		rret = dirreadent(*nf, buf + rlen, len - rlen);
		if(rret < 0)
			return rret;
		rlen += rret;
		*f = nf;
	}
	return rlen;
}

File *dirwalk(File *f, char const *name, size_t len)
{
	File **c = 0;
	while((c = dirnext(f, c))) {
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

void debugprintfile(File *f)
{
	if(f->st.st_mode & S_IFDIR) {
		/*dbgprintdir(f);*/
	} else {
		printf(" ");
	}
}

