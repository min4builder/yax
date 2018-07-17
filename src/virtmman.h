#ifndef _VIRTMMAN_H
#define _VIRTMMAN_H

#include <stddef.h>
#include <sys/types.h>
#include <yax/mapflags.h>
#include "ref.h"

enum pgfields {
	PGPRESENT = 1 << 0,
	PGWRITEABLE = 1 << 1,
	PGUSER = 1 << 2,
	PGGLOBAL = 1 << 8,
	PGNOSHARE = 1 << 9
};

#define PGADDR 0xfffff000

typedef uint32_t PgEntry;
typedef PgEntry PgDir[1024];

typedef struct {
	RefCounted refcount;
	unsigned int len;
	size_t delta;
	PgEntry *e;
} PgList;

/* in addition to yax/mapflags.h; internal kernel flag */
#define PROT_USER 8

PgDir *vpgnew(void);
void *vpgmap(void *, size_t, enum mapprot, enum mapflags, off_t);
PgDir *vpgcopy(int);
void vpgunmap(void *, size_t);
void vpgclear(void);
void vpgdel(PgDir *);

int verusrptr(const void *, size_t, enum mapprot);
int verusrstr(const char *, enum mapprot);

PgList *getusrptr(const void *, size_t);
void *putusrptr(PgList *);
void freeusrptr(PgList *, void *);

/* actually declared in boot.s; it needs it to be able to enable paging */
extern PgDir kernel_pd;

#endif /* _VIRTMMAN_H */

