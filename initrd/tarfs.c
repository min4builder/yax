#define __YAX__
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <yax/mount.h>
#include <yax/stat.h>
#include <codas/bit.h>
#include <yaxfs/dofunc.h>
#include <yaxfs/fid.h>
#include <yaxfs/file.h>
#include <yaxfs/serve.h>

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
	d.mode = (d.qid.type << 24) | (perm & ~0222);
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

static ssize_t tpread(Fid *fid, void *buf, size_t len, off_t off)
{
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
	Func func = { .pread = tpread };
	Fidpool fds = FIDPOOL;
	File *root = dirnew(mkdirent(0, 1, 0777, "/", "", "", 0));
	setupdir(root, "/", file);
	fidadd(&fds, fidnew(root, 0));
	for(;;) {
		Req r = recv(fd);
		dofunc(&r, &fds, &func);
		answer(r, fd);
	}
}

