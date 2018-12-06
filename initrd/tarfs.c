#define _BSD_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <yax/mount.h>
#include <yax/stat.h>
#include <yax/serve.h>
#include <codas/bit.h>
#include <yaxfs.h>

typedef struct {
	File *f;
	off_t off;
	int open;
} Fid;

static File *root;
static Fid fs[256];

static int oparse(char *s, int l)
{
	int r = 0;
	while(l-- > 0) {
		r <<= 3;
		r |= (*s++ - '0');
	}
	return r;
}

static const char *fname(const char *s)
{
	const char *os = s;
	while(*s) {
		if(*s == '/') {
			if(!s[1]) {
				char *ns = malloc(strlen(os));
				strlcpy(ns, os, strlen(os));
				return ns;
			} else
				os = s+1;
		}
		s++;
	}
	return os;
}

static int subent(char *a1, char *a2, char *b)
{
	char *a15 = "/";
	for(;;) {
		if(!*a1 && a15) {
			a1 = a15;
			a15 = a2;
			a2 = 0;
		}
		if(*a1 && !*b) {
			while(*a1) {
				if(*a1 == '/' && a1[1])
					return 0;
				a1++;
			}
			return 1;
		}
		if(*a1 != *b)
			return 0;
		a1++;
		b++;
	}
}

static Dir mkdirent(int j, int dir, int perm, const char *name, const char *uid, const char *gid, int size)
{
	Dir d = { { 0, 0, 0 }, 0, 0, 0, 0, "", "", "", "" };
	d.qid.path = j;
	d.qid.vers = 0;
	d.qid.type = dir ? QTDIR : 0;
	d.mode = (d.qid.type << 24) | perm;
	d.length = size;
	if(name[strlen(name)-1] == '/') {
		char *newname = malloc(strlen(name));
		strlcpy(newname, name, strlen(name));
		name = newname;
	}
	d.name = name;
	d.uid = uid;
	d.gid = gid;
	d.muid = d.uid;
	return d;
}

static void setupdir(File *dir, char *path, char *file)
{
	char *f = file;
	while(!memcmp(f + 257, "ustar", 5)) {
		int size = oparse(f + 124, 11);
		if(subent(f + 345, f, path)) {
			Dir d = mkdirent((int)f, f[156] == '5', oparse(f + 100, 6), fname(f), f + 265, f + 297, size);
			File *nf;
			if(f[156] == '5') {
				char *np = malloc(strlen(path) + strlen(d.name) + 2);
				strcpy(np, path);
				strcat(np, d.name);
				strcat(np, "/");
				nf = dirnew(d);
				setupdir(nf, np, file);
				free(np);
			} else {
				nf = filenew(d, f, 0);
			}
			diraddfile(dir, nf);
		}
		f += ((size + 1023) / 512) * 512;
	}
}

static void setupdirs(char *file)
{
	root = dirnew(mkdirent(0, 1, 0777, "/", "", "", 0));
	setupdir(root, "/", file);
}

static ssize_t tpread(Fid *fid, void *buf, size_t len, off_t off)
{
	if(!(fid->open & O_RDONLY))
		return -EACCES;
	if(off + len > fid->f->dir.length)
		len = fid->f->dir.length - off < 0 ? 0 : fid->f->dir.length - off;
	memcpy(buf, (char *) fid->f->aux + 512 + off, len);
	return len;
}

int tarfsmkmnt(int *b)
{
	Qid qid = { 0x80, 0, 0 };
	return mkmnt(b, qid);
}

void tarfsserve(int fd, char *file)
{
	setupdirs(file);
	fs[0].f = root;
	fs[0].open = 0;
	for(;;) {
		Req r = recv(fd);
		switch(r.fn) {
		case MAUTH:
			if(r.fid != 0) {
				r.ret = -EACCES;
				break;
			}
			if(r.len != 0) {
				r.ret = -EACCES;
				break;
			}
			r.ret = 0;
			break;
		case MDEL:
			fs[r.fid].f = 0;
			fs[r.fid].open = 0;
			break;
		case MDUP: {
			int x;
			for(x = 0; x < 256; x++) {
				if(fs[x].f == 0) {
					fs[x].f = fs[r.fid].f;
					fs[x].open = fs[r.fid].open;
					r.ret = x;
					break;
				}
			}
			break;
		}
		case MPREAD:
			r.ret = tpread(&fs[r.fid], r.buf, r.len, r.off);
			break;
		case MSEEK:
			switch(r.submsg) {
			case 0:
				fs[r.fid].off = r.off;
				break;
			case 1:
				fs[r.fid].off += r.off;
				break;
			case 2:
				fs[r.fid].off = fs[r.fid].f->dir.length + r.off;
				break;
			}
			r.ret = fs[r.fid].off;
			break;
		case MWALK: {
			File *f;
			if(r.len2 != 13) {
				r.ret = -EINVAL;
				break;
			}
			r.ret = -ENOENT;
			if(!(fs[r.fid].f->dir.mode & DMDIR)) {
				r.ret = -ENOTDIR;
			} else {
				f = dirwalk(fs[r.fid].f, r.buf, r.len);
				if(f) {
					PBIT8(r.buf2, f->dir.qid.type);
					PBIT32((char *)r.buf2+1, f->dir.qid.vers);
					PBIT64((char *)r.buf2+9, f->dir.qid.path);
					fs[r.fid].f = f;
					fs[r.fid].open = 0;
					r.ret = 0;
				}
			}
			break;
		}
		case MOPEN:
			if(r.submsg & (O_EXCL | O_WRONLY)) {
				r.ret = -EACCES;
			} else {
				fs[r.fid].open = r.submsg;
				r.ret = 0;
			}
			break;
		default:
			r.ret = -ENOSYS;
			break;
		}
		answer(r, fd);
	}
}

