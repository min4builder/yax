#ifndef _CONN_H
#define _CONN_H

#include <sys/lock.h>
#include <sys/ref.h>
#include <sys/types.h>
#include "mem/str.h"
#include "stat.h"

typedef struct Conn Conn;

typedef struct {
	int impl;
	void (*del)(Conn *);
	Conn *(*dup)(Conn *);
	long long (*f)(Conn *, int, int, void *, size_t, off_t);
} Dev;

struct Conn {
	RefCounted refcounted;
	Dev *dev;
	void *inst;
	Str *name;
	Qid qid;
};

enum {
	MWANTSPTR = 1 << 16,
	MWANTSOFF = 2 << 16,
	MWANTSWR = 4 << 16,
	MWANTSFILTER = 0xffff
};
enum {
	MDEL = 0,
	MDUP = 1,
	MWALK = 2 | MWANTSPTR,
	MOPEN = 3,
	MSREAD = 4 | MWANTSPTR | MWANTSWR,
	MSWRITE = 5 | MWANTSPTR,
	MSEEK = 6,
	MPREAD = 7 | MWANTSPTR | MWANTSWR | MWANTSOFF,
	MPWRITE = 8 | MWANTSPTR | MWANTSOFF,
	MSTAT = 9 | MWANTSPTR | MWANTSWR,
	MWSTAT = 10 | MWANTSPTR,
	MIOCTL = 11
};

#define MIMPL(n) (1 << ((n) & MWANTSFILTER))
#define MIMPLALL (~0)

void conninit(Conn *, const char *, Qid, Dev *, void *);
Conn *conndup(Conn *, Str *name);

long long connfunc(Conn *, int, int, void *, size_t, off_t);


#endif /* _CONN_H */

