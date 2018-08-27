#define NDEBUG
#include <stddef.h>
#include <stdint.h>
#include <yax/errorcodes.h>
#include <yax/mapflags.h>
#include "arch.h"
#include "boot.h"
#include "libk.h"
#include "malloc.h"
#include "pgdir.h"
#include "physmman.h"
#include "printk.h"
#include "ref.h"
#include "virtmman.h"

/* all code related to page directories; physical memory allocation is in
 * physmman.c */

static void map(uintptr_t pg, PgEntry);
static uintptr_t unmap(uintptr_t pg);

static PgEntry mkpg(uintptr_t addr, int prot, int noshare)
{
	return addr
		| (noshare ? PGNOSHARE : 0)
		| (addr >= (uintptr_t) VIRT(0) ? PGGLOBAL : PGUSER)
		| (prot & PROT_WRITE ? PGWRITEABLE : 0)
		| PGPRESENT;
}

PgDir *vpgnew(void)
{
	uintptr_t pdp = ppgalloc();
	PgDir *pd = SPG;
	ppgref(pdp);
	map((uintptr_t) SPG, mkpg(pdp, PROT_READ | PROT_WRITE, 1));
	memcpy(pd, &kernel_pd, sizeof(*pd));
	(*pd)[1023] = pdp | PGNOSHARE | PGWRITEABLE | PGPRESENT;
	unmap((uintptr_t) SPG);
	return (void *) pdp;
}

PgDir *vpgcopy(int memshare)
{
	unsigned int i, j;
	PgDir *ppd = vpgnew();
	for(i = 0; i < ((unsigned int) VIRT(0) / PGLEN) / 1024; i++) {
		PgDir *pt = PT(i);
		if(!((*PGDIR)[i] & PGPRESENT))
			continue;
		map((uintptr_t) SPG, mkpg((uintptr_t) ppd, PROT_READ | PROT_WRITE, 1));
		(*(PgDir *)SPG)[i] = (*PGDIR)[i] & ~PGADDR;
		(*(PgDir *)SPG)[i] |= ppgalloc();
		map((uintptr_t) SPG, mkpg((*(PgDir *)SPG)[i] & PGADDR, PROT_READ | PROT_WRITE, 1));
		for(j = 0; j < 1024; j++) {
			if(!((*pt)[j] & PGPRESENT)) {
				if(((*pt)[j] & 0xf) == PGEMPTY) {
					/* TODO FIXME empty pages not shared */
					(*(PgDir *)SPG)[j] = (*pt)[j];
				}
			} else if((*pt)[j] & PGWRITEABLE && (!memshare || (*pt)[j] & PGNOSHARE)) {
				(*pt)[j] = ((*pt)[j] & ~PGWRITEABLE) | PGCOW;
				(*(PgDir *)SPG)[j] = ((*pt)[j] & ~PGWRITEABLE) | PGCOW;
				ppgref((*pt)[j] & PGADDR);
			} else {
				(*(PgDir *)SPG)[j] = (*pt)[j];
				ppgref((*pt)[j] & PGADDR);
			}
		}
	}
	unmap((uintptr_t) SPG);
	return ppd;
}

static void map(uintptr_t pg, PgEntry page)
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
	}
	pt = PT((pg / PGLEN) / 1024);
	(*pt)[(pg / PGLEN) % 1024] = page;
	pginval(pg / PGLEN);
	printk(";\n");
}

static uintptr_t findpgs(unsigned int npages, int user)
{
	/* can't map at 0 */
	uintptr_t i = user ? PGLEN : (uintptr_t) VIRT(0);
	uintptr_t limit = user ? (uintptr_t) MAXUPG : (uintptr_t) SPG;
	while(i < limit) {
		uintptr_t j = i + npages * PGLEN;
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
		return j - npages * PGLEN;
contwhile:	;
	}
	return 0;
}

void *vpgmap(void *addr, size_t len, enum mapprot prot, enum mapflags flags, Conn *c, off_t off, size_t flen)
{
	unsigned int npgs = (len + PGLEN - 1) / PGLEN;
	uintptr_t pgs;
	uintptr_t i;
	if(flags & MAP_FIXED) {
		vpgunmap(addr, len);
		pgs = (uintptr_t) addr;
	} else {
		pgs = findpgs(npgs, prot & PROT_USER);
		if(pgs == 0)
			return ERR2PTR(-ENOMEM);
	}
	if(!(flags & (MAP_FIXED | MAP_ANONYMOUS))) {
		/* TODO */
	}
	for(i = pgs; i < pgs + len; i += PGLEN) {
		if(flags & MAP_PHYS) {
			map(i, mkpg(off, prot, flags & MAP_NOSHARE));
		} else if(flags & MAP_ANONYMOUS) {
			if(flags & MAP_NOSHARE) {
				map(i, PGEMPTY | (prot << 8) | ((flags & MAP_NOSHARE) << 16));
			} else {
				uintptr_t p = ppgalloc();
				map(i, mkpg(p, prot, flags & MAP_NOSHARE));
				memset((void *) i, 0xBE, PGLEN - i % PGLEN);
			}
		} else {
			/* TODO FIXME no backing */
			if(flags & MAP_PRIVATE) {
				uintptr_t p = ppgalloc();
				ssize_t ret;
				map(i, mkpg(p, prot | PROT_WRITE, flags & MAP_NOSHARE));
				if(pgs + flen - i > 0)
					ret = connpread(c, (void *) i, pgs + flen - i < PGLEN ? pgs + flen - i : PGLEN, off + i - pgs);
				else
					ret = 0;
				if(ret < 0)
					{/* TODO */}
				/* POSIX says it should be zero-filled */
				memset((void *) (i + ret), 0, PGLEN - ret - i % PGLEN);
				/* undirty the page and maybe remove write perm */
				map(i, mkpg(p, prot, flags & MAP_NOSHARE));
			}
		}
	}
	return (void *) pgs;
}

static uintptr_t unmap(uintptr_t pg)
{
	uintptr_t p = 0;
	PgEntry *pe;
	unsigned int i;
	if((*PGDIR)[(pg / PGLEN) / 1024] & PGPRESENT) {
		PgDir *pt = PT((pg / PGLEN) / 1024);
		pe = &(*pt)[(pg / PGLEN) % 1024];
		if(*pe & PGPRESENT) {
			p = *pe & PGADDR;
			if((*pe & (PGFILE | PGDIRTY)) == (PGFILE | PGDIRTY)) {
				/* TODO implement this */
				printk("Dirty file at ");
				uxprintk(pg);
				cprintk(' ');
				uxprintk((*PGDIR)[(pg / PGLEN) / 1024]);
				cprintk(' ');
				uxprintk(*pe);
				cprintk('\n');
				halt();
			}
			*pe = 0;
			pginval(pg / PGLEN);
		}
		for(i = 0; i < 1024; i++) {
			if((*pt)[i] & PGPRESENT) {
				return p;
			}
		}
		ppgunref((*PGDIR)[(pg / PGLEN) / 1024] & PGADDR);
		(*PGDIR)[(pg / PGLEN) / 1024] = 0;
		ptinval((pg / PGLEN) / 1024);
	}
	return p;
}

void vpgunmap(void *addr, size_t len)
{
	uintptr_t pgs = (uintptr_t) addr;
	uintptr_t i;
	for(i = pgs; i < pgs + len; i += PGLEN) {
		uintptr_t p = unmap(i);
		if(p) {
			ppgunref(p);
		}
	}
}

void vpgclear(void)
{
	vpgunmap(0, VIRT(0) - (uint8_t *) 1);
}

void vpgdel(PgDir *pd)
{
	ppgunref((uintptr_t) pd);
}

static int verusrpage(unsigned int p, enum mapprot prot)
{
	PgDir *pt;
	if(!((*PGDIR)[p / 1024] & PGPRESENT))
		return 0;
	pt = PT(p / 1024);
	if(!((*pt)[p % 1024] & PGPRESENT))
		return 0;
	if(!((*pt)[p % 1024] & PGUSER))
		return 0;
	if(prot & PROT_WRITE && !((*pt)[p % 1024] & PGWRITEABLE))
		return 0;
	return 1;
}

int verusrptr(const void *p, size_t len, enum mapprot prot)
{
	uintptr_t pgs = (uintptr_t) p;
	uintptr_t i;
	for(i = pgs; i < pgs + len; i += PGLEN) {
		if(!verusrpage(i / PGLEN, prot))
			return 0;
	}
	return 1;
}

int verusrstr(const char *s, enum mapprot prot)
{
	/* this is overly inneficient */
	do {
		if(!verusrpage((unsigned int) s / PGLEN, prot))
			return 0;
	} while(*s++ != '\0');
	return 1;
}

static void freeup(const RefCounted *rc)
{
	PgList *pl = (PgList *) rc;
	unsigned int i;
	for(i = 0; i < pl->len; i++)
		ppgunref(pl->e[i]);
	free(pl);
}

PgList *getusrptr(const void *p, size_t len)
{
	uintptr_t pgs = (uintptr_t) p;
	uintptr_t i;
	PgDir *pt;
	PgList *pl = calloc(1, sizeof(PgList) + sizeof(PgEntry) * (len / PGLEN));
	pl->e = (void *) ((char *) pl + sizeof(PgList));
	mkref(pl, freeup);
	pl->len = len;
	pl->delta = pgs % PGLEN;
	for(i = pgs; i < pgs + len; i += PGLEN) {
		/* TODO security */
		pt = PT((i / PGLEN) / 1024);
		pl->e[(i - pgs) / PGLEN] = (*pt)[(i / PGLEN) % 1024] & PGADDR;
		ppgref(pl->e[(i - pgs) / PGLEN]);
	}
	return pl;
}

void *putusrptr(PgList *pl)
{
	unsigned int plen = pl->len + PGLEN - pl->len % PGLEN;
	uintptr_t pgs = findpgs(plen, 1);
	uintptr_t i;
	if(pgs == 0)
		return 0;
	ref(pl);
	for(i = pgs; i < pgs + pl->len; i += PGLEN) {
		/* TODO security */
		map(i, mkpg(pl->e[(i - pgs) / PGLEN], PROT_USER | PROT_READ | PROT_WRITE, 1));
	}
	return (void *) (pgs + pl->delta);
}

void freeusrptr(PgList *pl, void *ptr)
{
	unsigned int npgs = pl->len;
	uintptr_t pgs = (uintptr_t) ptr;
	uintptr_t i;
	for(i = pgs; i < pgs + pl->len; i += PGLEN) {
		uintptr_t p = unmap(i);
		if(p)
			ppgunref(p);
	}
	unref(pl);
}

void dumpregs(Regs *);

void page_fault(Regs *r, void *addr, uint32_t err)
{
	PgEntry pe;
	printk("Page fault 0x");
	uxprintk((uint32_t)addr);
	printk(" PDE: ");
	uxprintk((*PGDIR)[(unsigned int) addr / PGLEN / 1024]);
	if((*PGDIR)[(unsigned int) addr / PGLEN / 1024] & PGPRESENT) {
		PgDir *pt = PT((unsigned int) addr / PGLEN / 1024);
		printk(" PTE: ");
		uxprintk((*pt)[(unsigned int) addr / PGLEN % 1024]);
		if((*pt)[(unsigned int) addr / PGLEN % 1024] & PGPRESENT) {
			if((*pt)[(unsigned int) addr / PGLEN % 1024] & PGCOW) {
				uintptr_t p = ppgalloc();
				PgEntry *pep = &(*pt)[(unsigned int) addr / PGLEN % 1024];
				uintptr_t p2 = *pep & PGADDR;
				*pep &= ~(PGADDR | PGCOW);
				*pep |= p | PGWRITEABLE;
				map((uintptr_t) SPG, mkpg(p2, PROT_READ, 1));
				memcpy((void *) (((uintptr_t) addr / PGLEN) * PGLEN), SPG, PGLEN);
				printk(" CoW\n");
				return;
			}
			pe = (*pt)[(unsigned int) addr / PGLEN % 1024];
		} else if(((*pt)[(unsigned int) addr / PGLEN % 1024] & 0xf) == PGEMPTY) {
			uintptr_t p = ppgalloc();
			pe = (*pt)[(unsigned int) addr / PGLEN % 1024];
			map(((uintptr_t) addr / PGLEN) * PGLEN, mkpg(p, (pe >> 8) & 0xf, pe >> 16));
			memset((void *) (((uintptr_t) addr / PGLEN) * PGLEN), 0xBE, PGLEN);
			printk(" AoD\n");
			return;
		}
	}
	cprintk('\n');
	dumpregs(r);
	if(!(err & 0x1)) {
		printk("-not present");
		if(!(pe & PGPRESENT))
			printk(" (the problem)");
	}
	if(err & 0x2) {
		printk("-on write");
		if(!(pe & PGWRITEABLE))
			printk(" (read only)");
	}
	if(!(err & 0x4))
		printk("-by kernel (esp above not valid)");
	else {
		printk("-by a process");
		if(!(pe & PGUSER))
			printk(" (forbidden)");
	}
	if(err & 0x8)
		printk("-reserved bit set");
	if(err & 0x10)
		printk("-on execute");
	printk("-at 0x");
	uxprintk(r->cs);
	cprintk(':');
	uxprintk(r->eip);
	if(addr < (void *) 4096)
		printk("-null pointer dereference");
	printk("-PDE 0x");
	uxprintk((uint32_t) &(*PGDIR)[(unsigned int) addr / PGLEN / 1024]);
	printk("-PT 0x");
	uxprintk((uint32_t) &(*PT((unsigned int) addr / PGLEN / 1024))[(unsigned int) addr / PGLEN % 1024]);
	cprintk('\n');
	halt();
}

