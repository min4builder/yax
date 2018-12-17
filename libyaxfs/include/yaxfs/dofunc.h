#ifndef YAXFS_DOFUNC_H_
#define YAXFS_DOFUNC_H_

#include <yax/stat.h>
#include <yaxfs/fid.h>
#include <yaxfs/file.h>
#include <yaxfs/serve.h>

typedef struct {
	ssize_t (*pwrite)(Fid *, void const *, size_t, off_t);
	ssize_t (*write)(Fid *, void const *, size_t);
	ssize_t (*pread)(Fid *, void *, size_t, off_t);
	ssize_t (*read)(Fid *, void *, size_t);
} Func;

void dofunc(Req *, Fidpool *, Func *);

#endif /* YAXFS_DOFUNC_H_ */

