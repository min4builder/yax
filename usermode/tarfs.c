#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <yax/errorcodes.h>
#include <yax/openflags.h>
#include <yax/stat.h>
#include "serve.h"
#include "syscall.h"

typedef struct {
	char *pos;
	int open;
	size_t size;
} File;
typedef struct {
	char *pos;
	Dir *de;
	int nde;
} Directory;

static File fs[256];
static Directory ds[256];

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
		if(*s == '/')
			os = s+1;
		s++;
	}
	return os;
}

static int sstrdir(const char *a1, const char *a2, const char *b1, const char *b2)
{
	char *a15 = "/", *b15 = "/";
	for(;;) {
		if(!*a1 && a15) {
			a1 = a15;
			a15 = a2;
			a2 = 0;
		}
		if(!*b1 && b15) {
			b1 = b15;
			b15 = b2;
			b2 = 0;
		}
		if(!*b1 && *a1) {
			while(*a1) {
				if(*a1 == '/')
					return 0;
				a1++;
			}
			return 1;
		}
		if(*a1 != *b1 || (!*a1 && !*b1))
			return 0;
		a1++;
		b1++;
	}
}

static void adddirent(int i, int j, char *f, int size)
{
	Dir d = { { 0, 0, 0 }, 0, 0, 0, 0, "", "", "", "" };
	d.qid.path = j;
	d.qid.vers = 0;
	d.qid.type = f[156] == '5' ? 0x80 : 0;
	d.mode = (d.qid.type << 24) | oparse(f + 100, 6);
	d.length = size;
	d.name = fname(f);
	d.uid = f + 265;
	d.gid = f + 297;
	d.muid = d.uid;
	ds[i].de[ds[i].nde++] = d;
	print("Adding a new dirent\n");
}

static void setupdirs(char *file)
{
	int dirn;
	char *dir = file;
	ds[0].pos = file; /* XXX FIXME */
	ds[0].de = malloc(256 * sizeof(Dir));
	ds[0].nde = 0;
	for(dirn = 0; !memcmp(dir + 257, "ustar", 5); dirn++) {
		int size = oparse(dir + 124, 11);
		print(dir + 345);
		print("/");
		print(dir);
		if(sstrdir(dir + 345, dir, "", ""))
			adddirent(0, dirn, dir, size);
		if(dir[156] == '5') {
			char *f = file;
			int i, j;
			for(i = 0; i < 256; i++) {
				if(!ds[i].pos) {
					ds[i].pos = dir;
					ds[i].de = malloc(256 * sizeof(Dir));
					ds[i].nde = 0;
					break;
				}
			}
			for(j = 0; !memcmp(f + 257, "ustar", 5); j++) {
				int size = oparse(f + 124, 11);
				if(sstrdir(f + 345, f, dir + 345, dir))
					adddirent(i, j, f, size);
				f += ((size + 1023) / 512) * 512;
			}
		}
		dir += ((size + 1023) / 512) * 512;
	}
}

static ssize_t tpread(File *f, void *buf, size_t len, off_t off)
{
	if(!(f->open & OREAD))
		return -EACCES;
	if(off + len > f->size)
		len = f->size - off < 0 ? 0 : f->size - off;
	memcpy(buf, f->pos + 512 + off, len);
	return len;
}

static int sstreq(const char *a1, const char *a2, const char *b1, const char *b2)
{
	char *a15 = "/", *b15 = "/";
	for(;;) {
		if(!*a1 && a15) {
			a1 = a15;
			a15 = a2;
			a2 = 0;
		}
		if(!*b1 && b15) {
			b1 = b15;
			b15 = b2;
			b2 = 0;
		}
		if(*a1 != *b1)
			return 0;
		if(!*a1 && !*b1)
			return 1;
		a1++;
		b1++;
	}
}

void tarfsserve(int fd, char *file)
{
	int i;
	setupdirs(file);
	for(i = 0; i < 256; i++) {
		if(ds[i].pos) {
			int j;
			print(ds[i].pos+345);
			print("/");
			print(ds[i].pos);
			print("{\n");
			for(j = 0; j < ds[i].nde; j++) {
				print("\t");
				print(ds[i].de[j].name);
				print("\n");
			}
		}
	}
	exits("");

#if 0
	/* TODO set this up as / */
	fs[0].open = 0;
	fs[0].pos = file;
	fs[0].size = 0;
	for(;;) {
		Req r = recv(fd);
		switch(r.fn) {
		case MSGDEL:
			fs[r.fid].pos = 0;
			fs[r.fid].open = 0;
			fs[r.fid].size = 0;
			break;
		case MSGDUP: {
			int x;
			for(x = 0; x < 256; x++) {
				if(fs[x].pos == 0) {
					fs[x].pos = fs[r.fid].pos;
					fs[x].open = fs[r.fid].open;
					fs[x].size = fs[r.fid].size;
					r.u.dup.ret = x;
					break;
				}
			}
			break;
		}
		case MSGPREAD:
			r.u.rw.ret = tpread(&fs[r.fid], r.u.rw.buf, r.u.rw.len, r.u.rw.off);
			break;
		case MSGSTAT:
			exits("broken");
			break;
		case MSGWALK: {
			char *pos;
			for(pos = file; pos < 
			break;
		case MSGOPEN:
			exits("broken");
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
#endif
}

