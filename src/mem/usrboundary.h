#ifndef _MEM_USRBOUNDARY_H
#define _MEM_USRBOUNDARY_H

#include <stddef.h>
#include <sys/types.h>
#include <codas/ref.h>
#include <yax/mapflags.h>
#include "mem/pgdir.h"

typedef struct {
	RefCounted refcount;
	unsigned int len;
	size_t delta;
	PgEntry *e;
} PgList;

int verusrptr(const void *, size_t, enum mapprot);
int verusrstr(const char *, enum mapprot);

PgList *getusrptr(const void *, size_t);
void *putusrptr(PgList *);
void freeusrptr(PgList *, void *);

#endif /* _MEM_USRBOUNDARY_H */

