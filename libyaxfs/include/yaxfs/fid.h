#ifndef YAXFS_FID_H_
#define YAXFS_FID_H_

#include <sys/types.h>
#include <yaxfs/file.h>

typedef struct {
	off_t off;
	char omode;
	File **dirread;
	File *f;
	void *aux;
} Fid;

typedef struct {
	Fid *fids[256];
} Fidpool;

#define FIDPOOL { { 0 } }

Fid *fidnew(File *, void *aux);
void fiddel(Fid *);
int fidadd(Fidpool *, Fid *);
Fid *fidrem(Fidpool *, int);
Fid *fidlookup(Fidpool *, int);

#endif /* YAXFS_FID_H_ */

