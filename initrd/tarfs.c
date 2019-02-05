#define __YAX__
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <yax/mount.h>
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

static struct stat mkdirent(int j, int dir, int perm, int size)
{
	struct stat st = { 0 };
	st.st_ino = j;
	st.st_mode = (dir ? S_IFDIR : 0) | (perm & ~0222);
	st.st_size = size;
	return st;
}

static void setupdir(File *dir, char *path, char *file)
{
	char *f = file;
	while(!memcmp(f + 257, "ustar", 5)) {
		int size = oparse(f + 124, 11);
		if(subent(f + 345, f, path)) {
			struct stat st = mkdirent((int)f, f[156] == '5', oparse(f + 100, 6), size);
			char const *name = fname(f);
			File *nf;
			if(f[156] == '5') {
				char *np = malloc(strlen(path) + strlen(name) + 2);
				strcpy(np, path);
				strcat(np, name);
				strcat(np, "/");
				nf = dirnew(st, name);
				setupdir(nf, np, file);
				free(np);
			} else {
				nf = filenew(st, name, f, 0);
			}
			diraddfile(dir, nf);
		}
		f += ((size + 1023) / 512) * 512;
	}
}

static ssize_t tpread(Fid *fid, void *buf, size_t len, off_t off)
{
	if(off + len > fid->f->st.st_size)
		len = fid->f->st.st_size - off < 0 ? 0 : fid->f->st.st_size - off;
	memcpy(buf, (char *) fid->f->aux + 512 + off, len);
	return len;
}

void tarfsserve(int fd, char *file)
{
	Func func = { .pread = tpread };
	Fidpool fds = FIDPOOL;
	File *root = dirnew(mkdirent(0, 1, 0777, 0), "/");
	setupdir(root, "/", file);
	fidadd(&fds, fidnew(root, 0));
	for(;;) {
		Req r = recv(fd);
		dofunc(&r, &fds, &func);
		answer(r, fd);
	}
}

