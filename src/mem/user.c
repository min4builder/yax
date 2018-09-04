#include <yax/mapflags.h>
#include "libk.h"
#include "mem/malloc.h"
#include "mem/phys.h"
#include "mem/ref.h"
#include "mem/user.h"
#include "printk.h"

static Pmap phys2page_ = { 0 };
static Pmap *phys2page = &phys2page_;

Page *pmapget(uintptr_t p)
{
	Page *cp;
	Pmap *pm = phys2page;
	int i;
	for(i = sizeof(uintptr_t) * 8 - 4; i >= 12; i -= 4) {
		cp = (*pm)[(p >> i) & 0xf];
		if(!cp)
			return 0;
		pm = (Pmap *) cp;
	}
	return cp;
}

void pmapset(uintptr_t p, Page *np)
{
	Page *cp;
	Pmap *pm = phys2page;
	int i;
	for(i = sizeof(uintptr_t) * 8 - 4; i >= 12; i -= 4) {
		cp = (*pm)[(p >> i) & 0xf];
		if(!cp && i > 12) {
			cp = malloc(sizeof(Pmap));
			memset(cp, 0, sizeof(Pmap));
			(*pm)[(p >> i) & 0xf] = cp;
		} else if(i == 12) {
			if(cp)
				unref(cp);
			(*pm)[(p >> i) & 0xf] = np;
			ref(np);
		}
		pm = (Pmap *) cp;
	}
}

void pmapdel(uintptr_t p)
{ (void) p; /* TODO */ }

static void freepage(const RefCounted *rc)
{
	Page *p = (Page *) rc;
	if(p->c)
		unref(p->c);
	if(p->p != (uintptr_t) NOPHYSPG)
		ppgfree(p->p);
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

