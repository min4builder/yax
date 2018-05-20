#ifndef _CONN_H
#define _CONN_H

#include <sys/types.h>
#include "lock.h"

typedef struct {
	int refcnt;
	Lock l;
	enum { CONNPIPE, CONNMUX } kind;
	char *name;
} Conn;

void conninit(Conn *, int, const char *);
void connref(Conn *);
void connunref(Conn *);

ssize_t connread(Conn *, void *, size_t);
ssize_t connwrite(Conn *, const void *, size_t);
off_t connseek(Conn *, off_t, int);

ssize_t connpread(Conn *, void *, size_t, off_t);
ssize_t connpwrite(Conn *, const void *, size_t, off_t);

ssize_t connstat(Conn *, void *, size_t);
ssize_t connwstat(Conn *, const void *, size_t);

#endif /* _CONN_H */

