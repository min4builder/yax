#define NDEBUG
#define __YAX__
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <codas/ref.h>
#include <yax/errorcodes.h>
#include <yax/mapflags.h>
#include "arch.h"
#include "boot.h"
#include "mem/malloc.h"
#include "mem/pgdir.h"
#include "mem/phys.h"
#include "mem/user.h"
#include "mem/virt.h"
#include "multitask.h"
#include "printk.h"

void *vpgspecmap(uintptr_t pg)
{
	PgDir *pt;
#if 0
	printk("vpgspecmap(");
	uxprintk(pg);
	printk(");\n");
#endif
	if(!((*PGDIR)[((uintptr_t) SPG / PGLEN) / 1024] & PGPRESENT)) {
		(printk)("SPG mapping space not allocated");
		halt();
	}
	pt = PT(((uintptr_t) SPG / PGLEN) / 1024);
	(*pt)[((uintptr_t) SPG / PGLEN) % 1024] = pg | PGNOSHARE | PGWRITEABLE | PGPRESENT;
	pginval((uintptr_t) SPG / PGLEN);
	return SPG;
}

void *vpgkmap(size_t len, enum mapprot prot)
{
	uintptr_t pgs, i;
	pgs = pgfind(len, 0);
	if(pgs == 0)
		return ERR2PTR(-ENOMEM);
	for(i = pgs; i < pgs + len + PGLEN-1; i += PGLEN) {
		uintptr_t p = ppgalloc();
		pgmap(i, pgemk(p, prot, 0, 0));
	}
	return (void *) pgs;
}

void *vpgumap(void *addr, size_t len, enum mapprot prot, enum mapflags flags)
{
	uintptr_t pgs, i;
	if(flags & MAP_FIXED) {
		vpgunmap(addr, len);
		pgs = (uintptr_t) addr;
	} else {
		pgs = pgfind(len, 1);
		if(pgs == 0)
			return ERR2PTR(-ENOMEM);
	}
	printk("umap(");
	uxprintk(pgs);
	printk(", ");
	uxprintk(len);
	printk(", ");
	iprintk(prot);
	printk(", ");
	iprintk(flags);
	printk(")\n");
	for(i = pgs; i < pgs + len + PGLEN-1; i += PGLEN) {
		Page *p = pagemk((uintptr_t) NOPHYSPG, prot, flags & MAP_NOSHARE);
		pgmap(i, (uintptr_t) p | PGNOTMAPPED);
		printk("i=");
		uxprintk(i);
		cprintk('\n');
	}
	return (void *) pgs;
}

void *vpgfmap(void *addr, size_t len, enum mapprot prot, enum mapflags flags, Conn *c, off_t off)
{
	uintptr_t pgs, i;
	if(flags & MAP_FIXED) {
		vpgunmap(addr, len);
		pgs = (uintptr_t) addr;
	} else {
		pgs = pgfind(len + off % PGLEN, 1);
		if(pgs == 0)
			return ERR2PTR(-ENOMEM);
	}
	printk("fmap(");
	uxprintk(pgs);
	printk(", ");
	uxprintk(len);
	printk(", ");
	iprintk(prot);
	printk(", ");
	iprintk(flags);
	printk(", ");
	uxprintk((uintptr_t) c);
	printk(", ");
	uxprintk(off);
	printk(")\n");
	for(i = pgs; i < pgs + len + PGLEN-1; i += PGLEN) {
		if(flags & MAP_PRIVATE) {
			Page *p = pagemk((uintptr_t) NOPHYSPG, prot, flags & MAP_NOSHARE);
			p->c = c;
			ref(c);
			p->off = off + i - pgs;
			p->start = i % PGLEN;
			p->len = pgs + len - i;
			if(p->len > PGLEN)
				p->len = PGLEN;
			pgmap(i, (uintptr_t) p | PGNOTMAPPED);
			printk("PDE: ");
			uxprintk((*PGDIR)[(i / PGLEN) / 1024]);
			printk(" PTE: ");
			uxprintk((uintptr_t) p | PGNOTMAPPED);
			printk(" i=");
			uxprintk(i);
			printk(" off=");
			uxprintk(p->off);
			printk(" start=");
			uxprintk(p->start);
			printk(" len=");
			uxprintk(p->len);
			cprintk('\n');
		} else {
			/* TODO */
		}
	}
	return (void *) pgs;
}

void *vpgpmap(void *addr, size_t len, enum mapprot prot, enum mapflags flags, uintptr_t phys)
{
	uintptr_t pgs, i;
	if(flags & MAP_FIXED) {
		vpgunmap(addr, len);
		pgs = (uintptr_t) addr;
	} else {
		pgs = pgfind(len, 1);
		if(pgs == 0)
			return ERR2PTR(-ENOMEM);
	}
	for(i = pgs; i < pgs + len + PGLEN-1; i += PGLEN) {
		pgmap(i, pgemk(phys, prot, flags & MAP_NOSHARE, 1));
	}
	return (void *) pgs;
}

void vpgunmap(void *addr, size_t len)
{
	uintptr_t pgs = (uintptr_t) addr;
	uintptr_t i;
	for(i = pgs; i < pgs + len + PGLEN-1; i += PGLEN) {
		uintptr_t p = pgunmap(i);
		if(p) {
			Page *vp = pmapget(p);
			if(!vp)
				ppgfree(p);
			else
				unref(vp);
		}
	}
}

PgDir *vpgnew(void)
{
	uintptr_t pdp = ppgalloc();
	PgDir *pd = SPG;
	vpgspecmap(pdp);
	memcpy(pd, &kernel_pd, sizeof(*pd));
	(*pd)[1023] = pdp | PGNOSHARE | PGWRITEABLE | PGPRESENT;
	return (void *) pdp;
}

PgDir *vpgcopy(int memshare)
{
	unsigned int i, j;
	PgDir *ppd = vpgnew();
	for(i = 0; i < ((unsigned int) VIRT(0) / PGLEN) / 1024; i++) {
		PgDir *pt = PT(i);
		uintptr_t pgdir;
		if(!((*PGDIR)[i] & PGPRESENT))
			continue;
		pgdir = ppgalloc();
		vpgspecmap((uintptr_t) ppd);
		(*(PgDir *)SPG)[i] = (*PGDIR)[i] & ~PGADDR;
		(*(PgDir *)SPG)[i] |= pgdir;
		printk("PDE: ");
		uxprintk((*(PgDir *)SPG)[i]);
		cprintk('\n');
		vpgspecmap((*(PgDir *)SPG)[i] & PGADDR);
		memset(SPG, 0, sizeof(PgDir));
		for(j = 0; j < 1024; j++) {
			if(!((*pt)[j] & PGPRESENT)) {
				if((*pt)[j] & PGNOTMAPPED) {
					/* TODO FIXME pages always shared */
					Page *p = (Page *) ((*pt)[j] & ~PGNOTMAPPED);
					ref(p);
					(*(PgDir *)SPG)[j] = (*pt)[j];
					uxprintk((i * 1024 + j) * PGLEN);
					printk(" AoD page=");
					uxprintk((uintptr_t) p);
					printk(" old PTE: ");
					uxprintk((*pt)[j]);
					printk(" new PTE: ");
					uxprintk((*(PgDir *)SPG)[j]);
					cprintk('\n');
				}
			} else if((*pt)[j] & PGPHYS) {
				(*(PgDir *)SPG)[j] = (*pt)[j];
				uxprintk((i * 1024 + j) * PGLEN);
				printk(" Phys old PTE: ");
				uxprintk((*pt)[j]);
				printk(" new PTE: ");
				uxprintk((*(PgDir *)SPG)[j]);
				cprintk('\n');
			} else if((*pt)[j] & PGWRITEABLE && (!memshare || (*pt)[j] & PGNOSHARE)) {
				Page *p;
				(*(PgDir *)SPG)[j] = (*pt)[j] = ((*pt)[j] & ~PGWRITEABLE) | PGCOW;
				p = pmapget((*pt)[j] & PGADDR);
				uxprintk((i * 1024 + j) * PGLEN);
				printk(" CoW page=");
				uxprintk((uintptr_t) p);
				printk(" old PTE: ");
				uxprintk((*pt)[j]);
				printk(" new PTE: ");
				uxprintk((*(PgDir *)SPG)[j]);
				cprintk('\n');
				ref(p);
			} else {
				Page *p;
				(*(PgDir *)SPG)[j] = (*pt)[j];
				p = pmapget((*pt)[j] & PGADDR);
				uxprintk((i * 1024 + j) * PGLEN);
				printk(" Shared page=");
				uxprintk((uintptr_t) p);
				printk(" old PTE: ");
				uxprintk((*pt)[j]);
				printk(" new PTE: ");
				uxprintk((*(PgDir *)SPG)[j]);
				cprintk('\n');
				ref(p);
			}
		}
	}
	return ppd;
}

void vpgclear(void)
{
	vpgunmap(0, VIRT(0) - (uint8_t *) 1);
}

void vpgdel(PgDir *pd)
{
	ppgfree((uintptr_t) pd);
}

static int ensure(uintptr_t pga, enum mapprot prot)
{
	PgEntry pe = (*PGDIR)[pga / PGLEN / 1024];
	PgDir *pt;
	if(!(pe & PGPRESENT))
		return -1;
	pt = PT(pga / PGLEN / 1024);
	pe = (*pt)[pga / PGLEN % 1024];
	if(pe & PGPRESENT) {
		if((pe & PGCOW) && (prot & PROT_WRITE)) {
			uintptr_t p2 = pe & PGADDR;
			Page *pg = pmapget(p2);
			uintptr_t p = ppgalloc();
			PgEntry *pep = &(*pt)[pga / PGLEN % 1024];
			Page *pg2 = pagemk(p, pg->prot, pg->noshare);
			pmapset(p, pg2);
			*pep &= ~(PGADDR | PGCOW);
			*pep |= p | PGWRITEABLE;
			vpgspecmap(p2);
			memcpy((void *) pga, SPG, PGLEN);
			(printk)("CoW new PTE: ");
			(uxprintk)(*pep);
			(printk)(" new Page: ");
			(uxprintk)((uintptr_t) pg2);
			(cprintk)('\n');
			return 0;
		} else if((pe & PGWRITEABLE) || !(prot & PROT_WRITE))
			return 0;
	} else if(pe & PGNOTMAPPED) {
		Page *pg = (Page *) (pe & ~PGNOTMAPPED);
		if(pg->p == (uintptr_t) NOPHYSPG) {
			pg->p = ppgalloc();
			pmapset(pg->p, pg);
			pe = pgemk(pg->p, pg->prot, pg->noshare, 0);
			pgmap(pga, pe | PGWRITEABLE);
			if(!pg->c)
				memset((void *) pga, 0, PGLEN);
			else {
				connfunc(pg->c, MPREAD, 0, (void *) (pga + pg->start), pg->len, pg->off);
				memset((void *) (pga + pg->start + pg->len), 0, PGLEN - (pg->len + pg->start));
			}
			if(!(pe & PGWRITEABLE))
				pgmap(pga, pe);
			(printk)(" AoD new PTE: ");
			(uxprintk)(pe);
			(cprintk)('\n');
		} else
			pgmap(pga, pgemk(pg->p, pg->prot, pg->noshare, 0));
		return 0;
	}
	return -1;
}

void dumpregs(Regs *);

void page_fault(Regs *r, void *addr, uint32_t err)
{
	PgEntry pe = 0;
	(printk)("Page fault 0x");
	(uxprintk)((uintptr_t)addr);
	(printk)(" PDE: ");
	(uxprintk)((*PGDIR)[(uintptr_t) addr / PGLEN / 1024]);
	if((*PGDIR)[(uintptr_t) addr / PGLEN / 1024] & PGPRESENT) {
		PgDir *pt = PT((uintptr_t) addr / PGLEN / 1024);
		(printk)(" PTE: ");
		pe = (*pt)[(uintptr_t) addr / PGLEN % 1024];
		(uxprintk)(pe);
		(cprintk)(' ');
		if(ensure(((uintptr_t) addr / PGLEN) * PGLEN, err & 0x2 ? PROT_READ | PROT_WRITE : PROT_READ) >= 0)
			return;
	}
	(cprintk)('\n');
	dumpregs(r);
	if(!(err & 0x1)) {
		(printk)("-not present");
		if(!(pe & PGPRESENT))
			(printk)(" (the problem)");
	}
	if(err & 0x2) {
		(printk)("-on write");
		if(!(pe & PGWRITEABLE))
			(printk)(" (read only)");
	}
	if(!(err & 0x4))
		(printk)("-by kernel (esp above not valid)");
	else {
		(printk)("-by process ");
		(uiprintk)(curproc->pid);
		if(!(pe & PGUSER))
			(printk)(" (forbidden)");
	}
	if(err & 0x8)
		(printk)("-reserved bit set");
	if(err & 0x10)
		(printk)("-on execute");
	if(addr < (void *) 4096)
		(printk)("-null pointer dereference");
	(printk)("-PDE 0x");
	(uxprintk)((uint32_t) &(*PGDIR)[(unsigned int) addr / PGLEN / 1024]);
	(printk)("-PT 0x");
	(uxprintk)((uint32_t) &(*PT((unsigned int) addr / PGLEN / 1024))[(unsigned int) addr / PGLEN % 1024]);
	(cprintk)('\n');
	halt();
}

