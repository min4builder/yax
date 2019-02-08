#define _POSIX_SOURCE
#include <stdlib.h>
#include <codas/vector.h>
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

void fiddel(Fid *f)
{
	free(f);
}

int fidadd(Fidpool *pool, Fid *f)
{
	unsigned fid;
	for(fid = 0; fid < veclen(pool); fid++) {
		if(!vecget(pool, fid)) {
			vecset(pool, fid, f);
			return fid;
		}
	}
	vecpush(pool, f);
	return veclen(pool) - 1;
}

Fid *fidrem(Fidpool *pool, int fid)
{
	Fid *const *f = vecget(pool, fid);
	Fid *z = 0;
	vecset(pool, fid, z);
	return *f;
}

Fid *fidlookup(Fidpool *pool, int fid)
{
	return *(Fid *const *) vecget(pool, fid);
}

