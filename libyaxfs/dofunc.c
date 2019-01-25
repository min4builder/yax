#define __YAX__
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <yax/errorcodes.h>
#include <codas/bit.h>
#include <yaxfs/dofunc.h>
#include <yaxfs/fid.h>
#include <yaxfs/file.h>
#include <yaxfs/serve.h>

void dofunc(Req *r, Fidpool *fp, Func *fs)
{
	Fid *fid = fidlookup(fp, r->fid);
	if(!fid) {
		r->ret = -EBADF;
		return;
	}
	switch(r->fn) {
	case MAUTH:
		/* TODO */
		r->ret = -EACCES;
		break;
	case MDEL:
		fidrem(fp, r->fid);
		fiddel(fid);
		r->ret = 0;
		break;
	case MDUP: {
		Fid *nfid = malloc(sizeof *nfid);
		memcpy(nfid, fid, sizeof *fid);
		r->ret = fidadd(fp, nfid);
		break;
	}
	default:
		deffunc(r, fid, fs);
		break;
	}
}

void deffunc(Req *r, Fid *fid, Func *fs)
{
	switch(r->fn) {
	case MOPEN:
		if(r->submsg & O_RDONLY && !(fid->f->st.st_mode & 0400))
			r->ret = -EACCES;
		else if(r->submsg & O_WRONLY && !(fid->f->st.st_mode & 0200))
			r->ret = -EACCES;
		else {
			fid->omode = r->submsg;
			r->ret = 0;
		}
		break;
	case MPWRITE:
		if(!(fid->omode & O_WRONLY)) {
			r->ret = -EACCES;
			break;
		}
		if(fs->pwrite)
			r->ret = fs->pwrite(fid, r->buf, r->len, r->off);
		else if(fs->write) {
			off_t off = fid->off;
			fid->off = r->off;
			r->ret = fs->write(fid, r->buf, r->len);
			fid->off = off;
		} else
			r->ret = -EINVAL;
		break;
	case MSWRITE:
		if(!(fid->omode & O_WRONLY)) {
			r->ret = -EACCES;
			break;
		}
		if(fs->write)
			r->ret = fs->write(fid, r->buf, r->len);
		else if(fs->pwrite) {
			r->ret = fs->pwrite(fid, r->buf, r->len, fid->off);
			if(r->ret > 0)
				fid->off += r->ret;
		}
		break;
	case MPREAD:
		if(!(fid->omode & O_RDONLY)) {
			r->ret = -EBADF;
			break;
		}
		if(fs->pread)
			r->ret = fs->pread(fid, r->buf, r->len, r->off);
		else if(fs->read) {
			off_t off = fid->off;
			fid->off = r->off;
			r->ret = fs->read(fid, r->buf, r->len);
			fid->off = off;
		} else
			r->ret = -EINVAL;
		break;
	case MSREAD:
		if(!(fid->omode & O_RDONLY)) {
			r->ret = -EBADF;
			break;
		}
		if(fs->read)
			r->ret = fs->read(fid, r->buf, r->len);
		else if(fs->pread) {
			r->ret = fs->pread(fid, r->buf, r->len, fid->off);
			if(r->ret > 0)
				fid->off += r->ret;
		} else
			r->ret = -EINVAL;
		break;
	case MSEEK:
		if(!fid->omode) {
			r->ret = -EBADF;
			break;
		}
		switch(r->submsg) {
		case 0:
			r->ret = fid->off = r->off;
			break;
		case 1:
			r->ret = fid->off += r->off;
			break;
		case 2:
			r->ret = fid->off = fid->f->st.st_size + r->off;
			break;
		default:
			r->ret = -EINVAL;
			break;
		}
		break;
	case MWALK: {
		File *new;
		if(r->len2 != 13) {
			r->ret = -EINVAL;
			break;
		}
		if(!(fid->f->st.st_mode & S_IFDIR)) {
			r->ret = -ENOTDIR;
			break;
		}
		new = dirwalk(fid->f, r->buf, r->len);
		if(!new)
			r->ret = -ENOENT;
		else {
			fid->f = new;
			PBIT64(r->buf2, new->st.st_ino);
			r->ret = 0;
		}
		break;
	}
	case MSTAT:
		r->ret = YAXstat2msg(&fid->f->st, r->buf, r->len);
		break;
	case MWSTAT:
		/* TODO */
		r->ret = -ENOSYS;
		break;
	default:
		r->ret = -ENOSYS;
		break;
	}
}

