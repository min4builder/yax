#ifndef YAXFS_FID_H_
#define YAXFS_FID_H_

#include <sys/types.h>
#include <yaxfs/file.h>
#include <codas/vector.h>

typedef struct {
	off_t off;
	char omode;
	File *const *dirread;
	File *f;
	void *aux;
} Fid;

typedef Vec(Fid *) Fidpool;

#define FIDPOOL vecnew(Fid *, 0)

Fid *fidnew(File *, void *aux);
void fiddel(Fid *);
int fidadd(Fidpool *, Fid *);
Fid *fidrem(Fidpool *, int);
Fid *fidlookup(Fidpool *, int);

#endif /* YAXFS_FID_H_ */

