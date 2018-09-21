#ifndef _CONN_H
#define _CONN_H

#include <sys/lock.h>
#include <sys/ref.h>
#include <sys/types.h>
#include "mem/str.h"
#include "stat.h"

typedef struct Conn Conn;

typedef struct {
	void (*del)(Conn *);
	Conn *(*dup)(Conn *);
	ssize_t (*pread)(Conn *, void *, size_t, off_t);
	ssize_t (*read)(Conn *, void *, size_t);
	ssize_t (*pwrite)(Conn *, const void *, size_t, off_t);
	ssize_t (*write)(Conn *, const void *, size_t);
	off_t (*seek)(Conn *, off_t, int);
	ssize_t (*stat)(Conn *, void *, size_t);
	ssize_t (*wstat)(Conn *, const void *, size_t);
	int (*walk)(Conn *, const char *path);
	int (*open)(Conn *, int, int);
} Dev;

struct Conn {
	RefCounted refcounted;
	Dev *dev;
	void *inst;
	Str *name;
	Qid qid;
};

void conninit(Conn *, const char *, Qid, Dev *, void *);

int connopen(Conn *, int, int);
Conn *conndup(Conn *, Str *name);
int connwalk(Conn *, const char *path);

ssize_t connread(Conn *, void *, size_t);
ssize_t connwrite(Conn *, const void *, size_t);
off_t connseek(Conn *, off_t, int);

ssize_t connpread(Conn *, void *, size_t, off_t);
ssize_t connpwrite(Conn *, const void *, size_t, off_t);

ssize_t connstat(Conn *, void *, size_t);
ssize_t connwstat(Conn *, const void *, size_t);

#endif /* _CONN_H */

