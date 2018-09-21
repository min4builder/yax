#define NDEBUG
#define __YAX__
#include <stdint.h>
#include <sys/types.h>
#include "arch.h"
#include "printk.h"
#include "boot.h"
#include "mem/pgdir.h"
#include "mem/phys.h"
#include "mem/virt.h"

/* all Region and PPage pointers are physical */

typedef struct Region Region;
struct Region {
	uintptr_t next;
	size_t len;
};

typedef struct PPage PPage;
struct PPage {
	uintptr_t next;
};

static uintptr_t rootrgn = NOPHYSPG;
static uintptr_t rootppg = NOPHYSPG;

static void showregions(void)
{
	uintptr_t r;
	Region *rgn;
	for(r = rootrgn; r != NOPHYSPG; r = rgn->next) {
		rgn = vpgspecmap(r);
		printk("Region from 0x");
		uxprintk((uintptr_t) r);
		printk(" to 0x");
		uxprintk((uintptr_t) r + rgn->len);
		cprintk('\n');
	}
}

void ppginit(int mml, MemoryMap *mm_)
{
	uint8_t *mmb = (void *)mm_;
	Region *r;
	while(mmb < (uint8_t *) mm_ + mml) {
		MemoryMap *mm = (void *)mmb;
		printk("rgn from 0x");
		uxprintk(mm->base);
		printk(" to 0x");
		uxprintk(mm->base + mm->len);
		if(mm->base % PGLEN) {
			mm->len -= PGLEN - mm->base % PGLEN;
			mm->base += PGLEN - mm->base % PGLEN;
			printk(" (base rounded to 0x");
			uxprintk(mm->base);
			printk(")");
		}
		if(mm->len % PGLEN) {
			mm->len -= mm->len % PGLEN;
			printk(" (limit rounded to 0x");
			uxprintk(mm->base + mm->len);
			printk(")");
		}
		/* todo split region if contains kernel */
		if(mm->type == 1 && mm->len && (mm->base + mm->len < KERNEL_LOAD || mm->base > KERNEL_LOAD_END)) {
			printk(" usable\n");
			r = vpgspecmap(mm->base);
			r->next = rootrgn;
			r->len = mm->len - mm->len % PGLEN;
			rootrgn = mm->base;
		} else {
			printk(" type ");
			iprintk(mm->type);
			cprintk('\n');
		}
		mmb += mm->size + sizeof(mm->size);
	}
	printk("kernel from 0x");
	uxprintk(KERNEL_LOAD);
	printk(" to 0x");
	uxprintk(KERNEL_LOAD_END);
	cprintk('\n');
	showregions();
}

uintptr_t ppgalloc(void)
{
	uintptr_t p;
	PPage *rppg;
	Region *rrgn;
	Region newr;
	if(rootppg != NOPHYSPG) {
		p = rootppg;
		rppg = vpgspecmap(rootppg);
		rootppg = rppg->next;
		printk("alloc() = ");
		uxprintk(p);
		printk(" rootppg=");
		uxprintk(rootppg);
		cprintk('\n');
		return p;
	}
	if(rootrgn == NOPHYSPG) {
		(printk)("Out of memory");
		halt();
	}
	p = rootrgn;
	rrgn = vpgspecmap(rootrgn);
	printk("alloc() = ");
	uxprintk(p);
	printk(" rgn len=");
	uxprintk(rrgn->len);
	printk(" rgn next=");
	uxprintk(rrgn->next);
	cprintk('\n');
	if(rrgn->len == PGLEN) {
		rootrgn = rrgn->next;
		return p;
	}
	newr = *rrgn;
	newr.len -= PGLEN;
	rootrgn += PGLEN;
	rrgn = vpgspecmap(rootrgn);
	*rrgn = newr;
	return p;
}

void ppgfree(uintptr_t p)
{
	PPage *pp = vpgspecmap(p);
	pp->next = rootppg;
	rootppg = p;
}

int ppgreserve(uintptr_t pg)
{
	uintptr_t pcp = NOPHYSPG, cp = rootrgn;
	while(cp != NOPHYSPG) {
		Region *r = vpgspecmap(cp);
		if(cp <= pg && pg < cp + r->len) {
			Region nr;
			nr.len = r->len - (pg - cp + PGLEN);
			nr.next = r->next;
			r->len = pg - cp;
			r->next = pg + PGLEN;
			if(r->len == 0) {
				if(pcp != NOPHYSPG) {
					r = vpgspecmap(pcp);
					r->next = pg + PGLEN;
				} else {
					rootrgn = pg + PGLEN;
				}
			}
			if(nr.len > 0) {
				r = vpgspecmap(pg + PGLEN);
				*r = nr;
			}
			return 0;
		}
		pcp = cp;
		cp = r->next;
	}
	pcp = NOPHYSPG;
	cp = rootppg;
	while(cp != NOPHYSPG) {
		PPage *pp = vpgspecmap(cp);
		if(cp == pg) {
			uintptr_t next = pp->next;
			if(pcp != NOPHYSPG) {
				pp = vpgspecmap(pcp);
				pp->next = next;
			} else {
				rootppg = next;
			}
			return 0;
		}
		pcp = cp;
		cp = pp->next;
	}
	return 1;
}

