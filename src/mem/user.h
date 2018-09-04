#ifndef _MEM_USER_H
#define _MEM_USER_H

#include <sys/types.h>
#include <yax/mapflags.h>
#include "conn.h"
#include "mem/ref.h"

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

typedef Page *Pmap[16];

Page *pmapget(uintptr_t);
void pmapset(uintptr_t, Page *);
void pmapdel(uintptr_t);
Page *pagemk(uintptr_t, enum mapprot, int);

#endif /* _MEM_USER_H */

