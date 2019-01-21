#define __YAX__
#include <stdlib.h>
#include <string.h>
#include <yax/stat.h>
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

File *dirnew(Dir d)
{
	Directory *dir = malloc(sizeof *dir);
	dir->nents = 0;
	dir->ents = 0;
	dir->cap = 0;
	return filenew(d, dir, freedir);
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

File *dirwalk(File *f, char const *name, size_t len)
{
	File **c = 0;
	while((c = dirnext(f, c))) {
		if(strncmp((*c)->dir.name, name, len) == 0)
			return *c;
	}
	return 0;
}

File *filenew(Dir d, void *aux, void (*freeaux)(File *))
{
	File *f = malloc(sizeof *f);
	f->lncnt = 0;
	f->dir = d;
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

