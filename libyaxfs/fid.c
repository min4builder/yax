#define __YAX__
#include <stdlib.h>
#include <yaxfs/fid.h>

Fid *fidnew(File *f, void *aux)
{
	Fid *fid = malloc(sizeof *fid);
	fid->off = 0;
	fid->omode = 0;
	fid->f = f;
	fid->dirread = 0;
	fid->aux = aux;
	return fid;
}

void fiddel(Fid *fid)
{
	free(fid);
}

int fidadd(Fidpool *pool, Fid *f)
{
	unsigned fid;
	for(fid = 0; fid < sizeof(pool->fids)/sizeof(pool->fids[0]); fid++) {
		if(!pool->fids[fid]) {
			pool->fids[fid] = f;
			return fid;
		}
	}
	return -1;
}

Fid *fidrem(Fidpool *pool, int fid)
{
	Fid *f = pool->fids[fid];
	pool->fids[fid] = 0;
	return f;
}

Fid *fidlookup(Fidpool *pool, int fid)
{
	return pool->fids[fid];
}

