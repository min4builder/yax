#define _YAX_
#include <stdlib.h>
#include <string.h>
#include <sys/serve.h>
#include <sys/types.h>
#include <unistd.h>
#include <yax/errorcodes.h>
#include <yax/openflags.h>
#include <yax/stat.h>

typedef struct File File;
struct File {
	Dir d;
	char *pos;
	File *sub;
	File *next;
};

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

static char *fname(const char *s)
{
	char *os = s;
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

static Dir mkdirent(int j, int dir, int perm, char *name, char *uid, char *gid, int size)
{
	Dir d = { { 0, 0, 0 }, 0, 0, 0, 0, "", "", "", "" };
	d.qid.path = j;
	d.qid.vers = 0;
	d.qid.type = dir ? 0x80 : 0;
	d.mode = (d.qid.type << 24) | perm;
	d.length = size;
	d.name = name;
	d.uid = uid;
	d.gid = gid;
	d.muid = d.uid;
	return d;
}

static void setupdir(File *d, char *path, char *file)
{
	char *f = file;
	while(!memcmp(f + 257, "ustar", 5)) {
		int size = oparse(f + 124, 11);
		if(subent(f + 345, f, path)) {
			File *nf = malloc(sizeof(File));
			nf->pos = f;
			nf->d = mkdirent((int)f, f[156] == '5', oparse(f + 100, 6), fname(f), f + 265, f + 297, size);
			nf->sub = 0;
			nf->next = d->sub;
			d->sub = nf;
			if(f[156] == '5') {
				char *np = malloc(strlen(path) + strlen(nf->d.name) + 2);
				strcpy(np, path);
				strcat(np, nf->d.name);
				strcat(np, "/");
				setupdir(nf, np, file);
				free(np);
			}
		}
		f += ((size + 1023) / 512) * 512;
	}
}

static void setupdirs(char *file)
{
	root = malloc(sizeof(File));
	root->pos = 0;
	root->d = mkdirent(0, 1, 0777, "/", "", "", 0);
	root->sub = 0;
	root->next = 0;
	setupdir(root, "/", file);
}

static ssize_t tpread(Fid *fid, void *buf, size_t len, off_t off)
{
	if(!(fid->open & OREAD))
		return -EACCES;
	if(off + len > fid->f->d.length)
		len = fid->f->d.length - off < 0 ? 0 : fid->f->d.length - off;
	memcpy(buf, fid->f->pos + 512 + off, len);
	return len;
}

void tarfsserve(int fd, char *file)
{
	setupdirs(file);
	fs[0].f = root;
	fs[0].open = 0;
	for(;;) {
		Req r = recv(fd);
		switch(r.fn) {
		case MSGDEL:
			fs[r.fid].f = 0;
			fs[r.fid].open = 0;
			break;
		case MSGDUP: {
			int x;
			for(x = 0; x < 256; x++) {
				if(fs[x].f == 0) {
					fs[x].f = fs[r.fid].f;
					fs[x].open = fs[r.fid].open;
					r.u.dup.ret = x;
					break;
				}
			}
			break;
		}
		case MSGREAD:
			r.u.rw.off = fs[r.fid].off;
		case MSGPREAD:
			r.u.rw.ret = tpread(&fs[r.fid], r.u.rw.buf, r.u.rw.len, r.u.rw.off);
			break;
		case MSGSEEK:
			switch(r.u.seek.whence) {
			case 0:
				fs[r.fid].off = r.u.seek.off;
				break;
			case 1:
				fs[r.fid].off += r.u.seek.off;
				break;
			case 2:
				fs[r.fid].off = fs[r.fid].f->d.length + r.u.seek.off;
				break;
			}
			r.u.seek.ret = fs[r.fid].off;
			break;
		case MSGSTAT:
			exits("broken");
			break;
		case MSGWALK: {
			File *f;
			r.u.walk.ret.type = 0xFF;
			r.u.walk.ret.path = -ENOENT;
			for(f = fs[r.fid].f->sub; f; f = f->next) {
				if(!strcmp(f->d.name, r.u.walk.path)) {
					fs[r.fid].f = f;
					r.u.walk.ret = f->d.qid;
					break;
				}
			}
			break;
		}
		case MSGOPEN:
			if(r.u.open.fl & (O_EXCL | O_WRONLY)) {
				r.u.open.ret = -EACCES;
			} else {
				fs[r.fid].open = r.u.open.fl;
				r.u.open.ret = 0;
			}
			break;
		case MSGPWRITE:
		case MSGWRITE:
			r.u.rw.ret = -EACCES;
			break;
		case MSGWSTAT:
			r.u.stat.ret = -EACCES;
			break;
		}
		answer(r, fd);
	}
}

