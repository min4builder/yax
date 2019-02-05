#ifndef YAXFS_FILE_H_
#define YAXFS_FILE_H_

#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct File File;
struct File {
	int lncnt;
	char const *name;
	struct stat st;
	void *aux;
	void (*freeaux)(File *);
};

File *dirnew(struct stat, char const *);
void diraddfile(File *, File *);
void dirdelfile(File *, File *);
File **dirnext(File *, File **);
File *dirwalk(File *, char const *, size_t);

ssize_t dirreadent(File *, char *, size_t);
ssize_t dirreadents(File *, File ***, char *, size_t);

File *filenew(struct stat, char const *, void *, void (*)(File *));
ssize_t filestat(File *, void *, size_t);
ssize_t filewstat(File *, void const *, size_t);

void filedel(File *);

#endif /* YAXFS_FILE_H_ */

