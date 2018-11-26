#ifndef _MEM_USER_H
#define _MEM_USER_H

#include <sys/types.h>
#include <codas/ref.h>
#include <yax/mapflags.h>
#include "fs/conn.h"

typedef struct {
	RefCounted rc;
	uintptr_t p;
	enum mapprot prot;
	int noshare;
	Conn *c;
	off_t off;
	size_t start;
	size_t len;
} Page;

typedef union Pmap Pmap;
union Pmap {
	Pmap *ps[16];
	Page *p;
};

Page *pmapget(uintptr_t);
void pmapset(uintptr_t, Page *);
void pmapdel(uintptr_t);
Page *pagemk(uintptr_t, enum mapprot, int);

#endif /* _MEM_USER_H */

