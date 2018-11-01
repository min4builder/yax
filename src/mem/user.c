#define NDEBUG
#define __YAX__
#include <string.h>
#include <sys/ref.h>
#include <yax/mapflags.h>
#include "mem/malloc.h"
#include "mem/phys.h"
#include "mem/user.h"
#include "printk.h"

static Pmap *phys2page = 0;

Page *pmapget(uintptr_t p)
{
	Pmap *pm = phys2page;
	int i;
	for(i = sizeof(uintptr_t) * 8 - 4; i >= 12; i -= 4) {
		if(!pm)
			return 0;
		pm = pm->ps[(p >> i) & 0xf];
	}
	printk("pmapget(");
	uxprintk(p);
	printk(") = ");
	uxprintk(pm ? (uintptr_t) pm->p : 0);
	cprintk('\n');
	return pm ? pm->p : 0;
}

void pmapset(uintptr_t p, Page *np)
{
	Pmap **up = &phys2page;
	Pmap *pm = phys2page;
	int i;
	printk("pmapset(");
	uxprintk(p);
	printk(", ");
	uxprintk((uintptr_t) np);
	printk(")\n");
	for(i = sizeof(uintptr_t) * 8 - 4; i >= 12 - 4; i -= 4) {
		if(i > 12 - 4 && !pm) {
			pm = malloc(sizeof(Pmap));
			memset(&pm->ps, 0, sizeof(Pmap*[16]));
			*up = pm;
		} else if(i == 12 - 4) {
			if(!pm) {
				pm = malloc(sizeof(Pmap));
				*up = pm;
			} else if(np && pm->p)
				unref(pm->p);
			pm->p = np;
			if(np)
				ref(np);
		}
		up = &(pm->ps[(p >> i) & 0xf]);
		pm = *up;
	}
}

void pmapdel(uintptr_t p)
{
	pmapset(p, 0);
}

static void freepage(const RefCounted *rc)
{
	Page *p = (Page *) rc;
	if(p->c)
		unref(p->c);
	if(p->p != (uintptr_t) NOPHYSPG) {
		pmapdel(p->p);
		ppgfree(p->p);
	}
	free(p);
}

Page *pagemk(uintptr_t pp, enum mapprot prot, int noshare)
{
	Page *p = malloc(sizeof(*p));
	if((uintptr_t) p & 0x3) {
		(printk)("Unaligned malloc");
		halt();
	}
	mkref(p, freepage);
	p->p = pp;
	p->prot = prot;
	p->noshare = noshare;
	p->c = 0;
	return p;
}

