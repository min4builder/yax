#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <yax/errorcodes.h>
#include <yax/openflags.h>
#include <yax/serve.h>
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
		print("Passing through ");
		print(f + 345);
		print("/");
		print(f);
		print("\n");
		if(subent(f + 345, f, path)) {
			File *nf = malloc(sizeof(File));
			nf->pos = f;
			nf->d = mkdirent((int)f, f[156] == '5', oparse(f + 100, 6), fname(f), f + 265, f + 297, size);
			nf->sub = 0;
			nf->next = d->sub;
			d->sub = nf;
			print("Found ");
			print(nf->d.name);
			print("\n");
			if(f[156] == '5') {
				char *np = malloc(strlen(path) + strlen(nf->d.name) + 2);
				print("Enter directory\n");
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

static void examine(File *f, int ind)
{
	int i;
	for(i = 0; i < ind; i++)
		print("\t");
	print(f->d.name);
	if(f->d.qid.type & 0x80) {
		File *de = f->sub;
		print(" {\n");
		for(de = f->sub; de != 0; de = de->next)
			examine(de, ind + 1);
		for(i = 0; i < ind; i++)
			print("\t");
		print("}");
	}
	print("\n");
}

void tarfsserve(int fd, char *file)
{
	setupdirs(file);
	examine(root, 0);
	fs[0].f = root;
	fs[0].open = 0;
	for(;;) {
		Req r = recv(fd);
		switch(r.fn) {
		case MSGDEL:
			print("del\n");
			fs[r.fid].f = 0;
			fs[r.fid].open = 0;
			break;
		case MSGDUP: {
			int x;
			print("dup\n");
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
		case MSGPREAD:
			print("pread\n");
			r.u.rw.ret = tpread(&fs[r.fid], r.u.rw.buf, r.u.rw.len, r.u.rw.off);
			break;
		case MSGSTAT:
			exits("broken");
			break;
		case MSGWALK: {
			File *f;
			print("walk\n");
			r.u.walk.ret = 0;
			for(f = fs[r.fid].f->sub; f; f = f->next) {
				if(!strcmp(f->d.name, r.u.walk.path)) {
					fs[r.fid].f = f;
					goto endcase;
				}
			}
			r.u.walk.ret = -ENOENT;
endcase:
			break;
		}
		case MSGOPEN:
			print("open\n");
			if(r.u.open.fl & (O_EXCL | O_WRONLY)) {
				r.u.open.ret = -EACCES;
			} else {
				fs[r.fid].open = r.u.open.fl;
				r.u.open.ret = 0;
			}
			break;
		case MSGPWRITE:
			r.u.rw.ret = -EACCES;
			break;
		case MSGWSTAT:
			r.u.stat.ret = -EACCES;
			break;
		}
		answer(r, fd);
	}
}

