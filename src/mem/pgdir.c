#define NDEBUG
#define __YAX__
#include <string.h>
#include <sys/types.h>
#include <codas/ref.h>
#include <yax/mapflags.h>
#include "mem/pgdir.h"
#include "mem/phys.h"
#include "mem/user.h"
#include "printk.h"

PgEntry pgemk(uintptr_t addr, enum mapprot prot, int noshare, int phys)
{
	return addr
		| (noshare ? PGNOSHARE : 0)
		| (phys ? PGPHYS : 0)
		| (addr >= (uintptr_t) VIRT(0) ? PGGLOBAL : PGUSER)
		| (prot & PROT_WRITE ? PGWRITEABLE : 0)
		| PGPRESENT;
}

void pgmap(uintptr_t pg, PgEntry page)
{
	PgDir *pt;
	printk("map(");
	uxprintk(pg);
	printk(", ");
	uxprintk(page);
	printk(")");
	if(!((*PGDIR)[(pg / PGLEN) / 1024] & PGPRESENT)) {
		unsigned int p = ppgalloc();
		printk(" alloc PT ");
		uxprintk(p);
		(*PGDIR)[(pg / PGLEN) / 1024] = p | PGUSER | PGWRITEABLE | PGPRESENT;
		ptinval((pg / PGLEN) / 1024);
		pginval((unsigned int) PT((pg / PGLEN) / 1024) / PGLEN);
		pt = PT((pg / PGLEN) / 1024);
		memset(pt, 0, sizeof(*pt));
	}
	pt = PT((pg / PGLEN) / 1024);
	(*pt)[(pg / PGLEN) % 1024] = page;
	pginval(pg / PGLEN);
	printk(";\n");
}

uintptr_t pgunmap(uintptr_t pg)
{
	uintptr_t p = 0;
	PgEntry *pe;
	unsigned int i;
	if((*PGDIR)[(pg / PGLEN) / 1024] & PGPRESENT) {
		PgDir *pt = PT((pg / PGLEN) / 1024);
		Page *up;
		pe = &(*pt)[(pg / PGLEN) % 1024];
		if(*pe & PGPRESENT) {
			printk("unmap(");
			uxprintk(pg);
			printk(");\n");
			p = *pe & PGADDR;
			if(*pe & PGDIRTY) {
				/* TODO implement this */
				printk("Dirty page at ");
				uxprintk(pg);
				printk(" PDE: ");
				uxprintk((*PGDIR)[(pg / PGLEN) / 1024]);
				printk(" PTE: ");
				uxprintk(*pe);
				cprintk('\n');
			}
			*pe = 0;
			pginval(pg / PGLEN);
		}
		for(i = 0; i < 1024; i++) {
			if((*pt)[i]) {
				return p;
			}
		}
		up = pmapget((*PGDIR)[(pg / PGLEN) / 1024] & PGADDR);
		if(up)
			unref(up);
		(*PGDIR)[(pg / PGLEN) / 1024] = 0;
		ptinval((pg / PGLEN) / 1024);
	}
	return p;
}

uintptr_t pgfind(size_t len, int user)
{
	/* can't find pages at 0 */
	uintptr_t i = user ? PGLEN : (uintptr_t) VIRT(0);
	uintptr_t limit = user ? (uintptr_t) MAXUPG : (uintptr_t) SPG;
	while(i < limit) {
		uintptr_t j = i + len;
		if(j >= limit)
			break;
		for(; i < j; i += PGLEN) {
			if((*PGDIR)[(i / PGLEN) / 1024] & PGPRESENT) {
				PgDir *pt = PT((i / PGLEN) / 1024);
				if((*pt)[(i / PGLEN) % 1024] & PGPRESENT) {
					i += PGLEN;
					goto contwhile;
				}
			}
		}
		return j - len;
contwhile:	;
	}
	return 0;
}

