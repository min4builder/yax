#ifndef YAXFS_FILE_H_
#define YAXFS_FILE_H_

#include <stddef.h>
#include <yax/stat.h>

typedef struct File File;
struct File {
	int lncnt;
	Dir dir;
	void *aux;
	void (*freeaux)(File *);
};

File *dirnew(Dir);
void diraddfile(File *, File *);
void dirdelfile(File *, File *);
File **dirnext(File *, File **);
File *dirwalk(File *, char const *, size_t);

File *filenew(Dir, void *, void (*)(File *));
ssize_t filestat(File *, void *, size_t);
ssize_t filewstat(File *, void const *, size_t);

void filedel(File *);

#endif /* YAXFS_FILE_H_ */

