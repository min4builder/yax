#ifndef _CONN_H
#define _CONN_H

#include <sys/types.h>
#include <codas/ref.h>
#include <yax/fn.h>
#include <yax/lock.h>
#include "mem/str.h"
#include "stat.h"

typedef struct Conn Conn;

typedef struct {
	void (*del)(Conn *);
	Conn *(*dup)(Conn *);
	long long (*f)(Conn *, int, int, void *, size_t, void *, size_t, off_t);
} Dev;

struct Conn {
	RefCounted refcounted;
	Dev *dev;
	void *inst;
	Str *name;
	Qid qid;
};

void conninit(Conn *, const char *, Qid, Dev *, void *);
Conn *conndup(Conn *, Str *);

int connsend(Conn *, int, int, void *, size_t, void *, size_t, off_t);
long long connrecv(Conn *, int);

long long connfunc(Conn *, int, int, void *, size_t, off_t);
long long connfuncpp(Conn *, int, int, void *, size_t, void *, size_t, off_t);

#endif /* _CONN_H */

