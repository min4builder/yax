#ifndef _MEM_PGDIR_H
#define _MEM_PGDIR_H

#include <sys/types.h>
#include <yax/mapflags.h>
#include "boot.h"

enum pgfields {
	PGPRESENT = 1 << 0,
	PGWRITEABLE = 1 << 1,
	PGUSER = 1 << 2,
	PGDIRTY = 1 << 6,
	PGGLOBAL = 1 << 8,
	PGNOSHARE = 1 << 9,
	PGCOW = 1 << 10,
	PGADDR = 0xfffff000
};

enum pgspecial {
	PGNOTMAPPED = 0x2
};

#define PGCNT ((uintptr_t) 1 << 20)
#define MAXUPG VIRT(0)
#define PGLEN ((uintptr_t) 1 << 12)

#define PGDIR ((PgDir *) 0xfffff000)
#define PT(x) ((PgDir *) (0xffc00000 + 0x1000 * (x)))
#define SPG ((void *) 0xffbfd000)

#define NOPHYSPG ((uintptr_t) ~0)

typedef uint32_t PgEntry;
typedef PgEntry PgDir[1024];

PgEntry pgemk(uintptr_t, enum mapprot, int);
void pgmap(uintptr_t, PgEntry);
uintptr_t pgunmap(uintptr_t);
uintptr_t pgfind(size_t, int);

void pginval(unsigned int pg);
void ptinval(unsigned int pt);
PgDir *getpgdir(void);
PgDir *switchpgdir(PgDir *);

/* actually declared in boot.s; it needs it to be able to enable paging */
extern PgDir kernel_pd;

#endif /* _MEM_PGDIR_H */

