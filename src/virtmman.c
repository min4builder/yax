#include <stddef.h>
#include <stdint.h>
#include <yax/mapflags.h>
#include "arch.h"
#include "boot.h"
#include "libk.h"
#include "pgdir.h"
#include "physmman.h"
#include "printk.h"
#include "virtmman.h"

static void *spg = (void *) (SPG * PGLEN);

static void map(unsigned int pg, unsigned int addr, int prot, int noshare);
static unsigned int unmap(unsigned int pg);

PgDir *vpgnew(void)
{
	unsigned int pdp = ppgalloc();
	PgDir *pd = spg;
	map(SPG, pdp, PROT_READ | PROT_WRITE, 1);
	memcpy(pd, &kernel_pd, sizeof(*pd));
	(*pd)[1023] = (pdp << 12) | PGNOSHARE | PGWRITEABLE | PGPRESENT;
	unmap(SPG);
	return (void *) (pdp * PGLEN);
}

PgDir *vpgcopy(int memshare)
{
	unsigned int i, j;
	PgDir *ppd = vpgnew();
	for(i = 0; i < ((unsigned int) VIRT(0) / PGLEN) / 1024; i++) {
		if(!((*PGDIR)[i] & PGPRESENT))
			continue;
		map(SPG, (unsigned int) ppd / PGLEN, PROT_READ | PROT_WRITE, 1);
		(*(PgDir *)spg)[i] = (*PGDIR)[i] & ~PGADDR;
		(*(PgDir *)spg)[i] |= ppgalloc() << 12;
		for(j = 0; j < 1024; j++) {
			PgDir *pt = PT(i);
			if(!((*pt)[j] & PGPRESENT))
				continue;
			if((*pt)[j] & PGNOSHARE || (!memshare && (*pt)[j] & PGWRITEABLE)) {
				unsigned int p = ppgalloc();
				map(SPG, (*(PgDir *)spg)[i] >> 12, PROT_READ | PROT_WRITE, 1);
				(*(PgDir *)spg)[j] = (*pt)[j] & ~PGADDR;
				(*(PgDir *)spg)[j] |= p << 12;
				map(SPG, p, PROT_READ | PROT_WRITE, 1);
				memcpy(spg, (void *) ((i * 1024 + j) * PGLEN), PGLEN);
			} else {
				map(SPG, (*(PgDir *)spg)[i] >> 12, PROT_READ | PROT_WRITE, 1);
				(*(PgDir *)spg)[j] = (*pt)[j];
				ppgref((*pt)[j] >> 12);
			}
		}
	}
	unmap(SPG);
	return ppd;
}

static void ensurepdmap(unsigned int pg)
{
	if(!((*PGDIR)[pg / 1024] & PGPRESENT)) {
		unsigned int p = ppgalloc();
		(*PGDIR)[pg / 1024] = p << 12 | PGUSER | PGWRITEABLE | PGPRESENT;
		ptinval(pg / 1024);
		pginval((unsigned int) PT(pg / 1024) / PGLEN);
	}
}

static void map(unsigned int pg, unsigned int addr, int prot, int noshare)
{
	PgDir *pt;
	ensurepdmap(pg);
	pt = PT(pg / 1024);
	(*pt)[pg % 1024] = addr << 12
		| (noshare ? PGNOSHARE : 0)
		| (pg >= (unsigned int) VIRT(0) / PGLEN ? PGGLOBAL : PGUSER)
		| (prot & PROT_WRITE ? PGWRITEABLE : 0)
		| PGPRESENT;
	pginval(pg);
}

static unsigned int findpgs(PgDir *pd, unsigned int npages, int user)
{
	/* can't map at 0 */
	unsigned int i = user ? 1 : (unsigned int) VIRT(0) / PGLEN;
	unsigned int limit = user ? UPGCNT : SPG;
	while(i < limit) {
		unsigned int j = i + npages;
		if(j >= limit)
			break;
		for(; i < j; i++) {
			if((*pd)[i / 1024] & PGPRESENT) {
				PgDir *pt = PT(i / 1024);
				if((*pt)[i % 1024] & PGPRESENT) {
					i++;
					goto contwhile;
				}
			}
		}
		return j - npages;
contwhile:	;
	}
	return 0;
}

void *vpgmap(void *addr, size_t len, enum mapprot prot, enum mapflags flags, off_t offs)
{
	unsigned int npgs = (len + PGLEN - 1) / PGLEN;
	unsigned int pgs;
	unsigned int i;
	if(flags & MAP_FIXED) {
		vpgunmap(addr, len);
		pgs = (unsigned int) addr / PGLEN;
	} else {
		pgs = findpgs(PGDIR, npgs, prot & PROT_USER);
		printk("pgs=0x");
		uxprintk(pgs);
		printk(" npgs=0x");
		uxprintk(npgs);
		cprintk('\n');
	}
	if(pgs == 0)
		return 0;
	for(i = pgs; i < pgs + npgs; i++) {
		if(flags & MAP_PHYS) {
			map(i, (unsigned int) offs / PGLEN, prot, flags & MAP_NOSHARE);
		} else if(flags & MAP_ANONYMOUS) {
			unsigned int p = ppgalloc();
			map(i, p, prot, flags & MAP_NOSHARE);
		}
	}
	return (void *) (pgs * PGLEN);
}

static unsigned int unmap(unsigned int pg)
{
	unsigned int p = 0;
	unsigned int i;
	if((*PGDIR)[pg / 1024] & PGPRESENT) {
		PgDir *pt = PT(pg / 1024);
		if((*pt)[pg % 1024] & PGPRESENT) {
			p = (*pt)[pg % 1024] >> 12;
			(*pt)[pg % 1024] = 0;
			pginval(pg);
		}
		for(i = 0; i < 1024; i++) {
			if((*pt)[i] & PGPRESENT)
				return p;
		}
		(*PGDIR)[pg / 1024] = 0;
		ppgunref((*PGDIR)[pg / 1024] >> 12);
		ptinval(pg / 1024);
	}
	return p;
}

void vpgunmap(void *addr, size_t len)
{
	unsigned int npgs = (len + PGLEN - 1) / PGLEN;
	unsigned int pgs = (unsigned int) addr / PGLEN;
	unsigned int i;
	for(i = pgs; i < pgs + npgs; i++) {
		unsigned int p = unmap(i);
		if(p)
			ppgunref(p);
	}
}

void vpgclear(void)
{
	vpgunmap(0, VIRT(0) - (uint8_t *) 1);
}

void vpgdel(PgDir *pd)
{
	ppgunref((unsigned int) pd / PGLEN);
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
	unsigned int npgs = (len + PGLEN - 1) / PGLEN;
	unsigned int pgs = (unsigned int) p / PGLEN;
	unsigned int i;
	for(i = pgs; i < pgs + npgs; i++) {
		if(!verusrpage(i, prot))
			return 0;
	}
	return 1;
}

int verusrstr(const char *s, enum mapprot prot)
{
	do {
		if(!verusrpage((unsigned int) s / PGLEN, prot))
			return 0;
		s++;
	} while(*s != '\0');
	return 1;
}

