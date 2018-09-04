#define NDEBUG
#include <stddef.h>
#include <stdint.h>
#include <yax/errorcodes.h>
#include <yax/mapflags.h>
#include "arch.h"
#include "boot.h"
#include "libk.h"
#include "mem/malloc.h"
#include "mem/pgdir.h"
#include "mem/phys.h"
#include "mem/user.h"
#include "mem/ref.h"
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
	for(i = pgs; i < pgs + len; i += PGLEN) {
		uintptr_t p = ppgalloc();
		pgmap(i, pgemk(p, prot, 0));
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
	for(i = pgs; i < pgs + len; i += PGLEN) {
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
	for(i = pgs; i < pgs + len; i += PGLEN) {
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
	for(i = pgs; i < pgs + len; i += PGLEN) {
		pgmap(i, pgemk(phys, prot, flags & MAP_NOSHARE));
	}
	return (void *) pgs;
}

void vpgunmap(void *addr, size_t len)
{
	uintptr_t pgs = (uintptr_t) addr;
	uintptr_t i;
	for(i = pgs; i < pgs + len; i += PGLEN) {
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

void dumpregs(Regs *);

void page_fault(Regs *r, void *addr, uint32_t err)
{
	PgEntry pe;
	(printk)("Page fault 0x");
	(uxprintk)((uint32_t)addr);
	(printk)(" PDE: ");
	(uxprintk)((*PGDIR)[(unsigned int) addr / PGLEN / 1024]);
	if((*PGDIR)[(unsigned int) addr / PGLEN / 1024] & PGPRESENT) {
		PgDir *pt = PT((unsigned int) addr / PGLEN / 1024);
		(printk)(" PTE: ");
		(uxprintk)((*pt)[(unsigned int) addr / PGLEN % 1024]);
		if((*pt)[(unsigned int) addr / PGLEN % 1024] & PGPRESENT) {
			if((*pt)[(unsigned int) addr / PGLEN % 1024] & PGCOW) {
				uintptr_t p = ppgalloc();
				PgEntry *pep = &(*pt)[(unsigned int) addr / PGLEN % 1024];
				uintptr_t p2 = *pep & PGADDR;
				*pep &= ~(PGADDR | PGCOW);
				*pep |= p | PGWRITEABLE;
				vpgspecmap(p2);
				memcpy((void *) (((uintptr_t) addr / PGLEN) * PGLEN), SPG, PGLEN);
				(printk)(" CoW new PTE: ");
				(uxprintk)(*pep);
				(cprintk)('\n');
				return;
			}
			pe = (*pt)[(unsigned int) addr / PGLEN % 1024];
		} else if((*pt)[(unsigned int) addr / PGLEN % 1024] & PGNOTMAPPED) {
			Page *pg = (Page *) ((*pt)[(unsigned int) addr / PGLEN % 1024] & ~PGNOTMAPPED);
			if(pg->p == (uintptr_t) NOPHYSPG) {
				pg->p = ppgalloc();
				pmapset(pg->p, pg);
				pe = pgemk(pg->p, pg->prot, pg->noshare);
				pgmap(((uintptr_t) addr / PGLEN) * PGLEN, pe | PGWRITEABLE);
				if(!pg->c)
					memset((void *) (((uintptr_t) addr / PGLEN) * PGLEN), 0xBE, PGLEN);
				else {
					connpread(pg->c, (void *) (((uintptr_t) addr / PGLEN) * PGLEN + pg->start), pg->len, pg->off);
					memset((void *) (((uintptr_t) addr / PGLEN) * PGLEN) + pg->start + pg->len, 0, PGLEN - (pg->len + pg->start));
				}
				if(!(pe & PGWRITEABLE))
					pgmap(((uintptr_t) addr / PGLEN) * PGLEN, pe);
				(printk)(" AoD new PTE: ");
				(uxprintk)(pe);
				(cprintk)('\n');
			} else
				pgmap(((uintptr_t) addr / PGLEN) * PGLEN, pgemk(pg->p, pg->prot, pg->noshare));
			return;
		}
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
	(printk)("-at 0x");
	(uxprintk)(r->cs);
	(cprintk)(':');
	(uxprintk)(r->eip);
	if(addr < (void *) 4096)
		(printk)("-null pointer dereference");
	(printk)("-PDE 0x");
	(uxprintk)((uint32_t) &(*PGDIR)[(unsigned int) addr / PGLEN / 1024]);
	(printk)("-PT 0x");
	(uxprintk)((uint32_t) &(*PT((unsigned int) addr / PGLEN / 1024))[(unsigned int) addr / PGLEN % 1024]);
	(cprintk)('\n');
	halt();
}

