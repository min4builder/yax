#define NDEBUG
#include <stdint.h>
#include <sys/types.h>
#include "arch.h"
#include "printk.h"
#include "boot.h"
#include "physmman.h"

/* this is just stupid; we need a better system */
static uint8_t pgs[PGCNT];
static uint32_t usable[PGCNT / 32];
static uint32_t reserved[PGCNT / 32];
static uintptr_t firstfree;
static uintptr_t lastusable;

static int isusable(uintptr_t pg)
{
	return usable[(pg / PGLEN) >> 5] & (1 << ((pg / PGLEN) & 0x1F)) && !(reserved[(pg / PGLEN) >> 5] & (1 << ((pg / PGLEN) & 0x1F)));
}

static void showregions(void)
{
	uint64_t i = 0, start;
	while(i < PGCNT * PGLEN - 1) {
		while(i < PGCNT * PGLEN - 1 && !isusable(i))
			i += PGLEN;
		start = i;
		while(i < PGCNT * PGLEN - 1 && isusable(i))
			i += PGLEN;
		printk("usable from 0x");
		uxprintk(start);
		printk(" to 0x");
		uxprintk(i);
		cprintk('\n');
	}
}

void ppginit(int mml, MemoryMap *mm_)
{
	uint64_t pg;
	uint8_t *mmb = (void *)mm_;
	while(mmb < (uint8_t *) mm_ + mml) {
		MemoryMap *mm = (void *)mmb;
		printk("rgn from 0x");
		uxprintk(mm->base);
		printk(" to 0x");
		uxprintk(mm->base + mm->len);
		if(mm->type == 1) {
			printk(" usable\n");
			for(pg = mm->base; pg < mm->base + mm->len; pg += PGLEN)
				usable[((uintptr_t) pg / PGLEN) >> 5] |= (1 << (((uintptr_t) pg / PGLEN) & 0x1F));
		} else {
			printk(" type ");
			iprintk(mm->type);
			cprintk('\n');
			for(pg = mm->base; pg < mm->base + mm->len; pg += PGLEN)
				reserved[((uintptr_t) pg / PGLEN) >> 5] |= (1 << (((uintptr_t) pg / PGLEN) & 0x1F));
		}
		mmb += mm->size + sizeof(mm->size);
	}
	printk("kernel from 0x");
	uxprintk(KERNEL_LOAD);
	printk(" to 0x");
	uxprintk(KERNEL_LOAD_END);
	cprintk('\n');
	for(pg = KERNEL_LOAD; pg < KERNEL_LOAD_END; pg += PGLEN)
		reserved[((uintptr_t) pg / PGLEN) >> 5] |= (1 << (((uintptr_t) pg / PGLEN) & 0x1F));
	for(pg = 0; pg < PGCNT * PGLEN - 1; pg += PGLEN) {
		if(isusable(pg)) {
			firstfree = pg;
			break;
		}
	}
	for(pg = 0; pg < PGCNT * PGLEN - 1; pg += PGLEN) {
		if(isusable(pg))
			lastusable = pg;
	}
	showregions();
}

uintptr_t ppgalloc(void)
{
	uintptr_t i;
	uintptr_t pg = firstfree;
	if(pgs[pg / PGLEN] != 0) {
		printk("NOMEM\n");
		halt();
	}
	pgs[pg / PGLEN]++;
	for(i = firstfree; i < PGCNT * PGLEN - 1; i += PGLEN) {
		if(pgs[i / PGLEN] == 0 && isusable(i)) {
			firstfree = i;
			break;
		}
	}
	return pg;
}

void ppgref(uintptr_t pg)
{
	printk("ppgref(");
	uxprintk(pg);
	printk(");\n");
	if(pgs[pg / PGLEN] == 0 || pgs[pg / PGLEN] == 0xFF) {
		printk("Page ");
		uxprintk(pg);
		printk(" wrongly referenced\n");
		halt();
	}
	pgs[pg / PGLEN]++;
}

void ppgunref(uintptr_t pg)
{
	printk("ppgunref(");
	uxprintk(pg);
	printk(");\n");
	if(pgs[pg / PGLEN] == 0) {
		printk("Page ");
		uxprintk(pg);
		printk(" underreferenced\n");
		halt();
	}
	pgs[pg / PGLEN]--;
	if(pgs[pg / PGLEN] == 0 && firstfree > pg && isusable(pg))
		firstfree = pg;
}

void ppgreserve(uintptr_t pg)
{
	pgs[pg / PGLEN]++;
	if(pgs[pg] != 1 || isusable(pg)) {
		printk("Page ");
		uxprintk(pg);
		printk(" was reserved or usable\n");
	}
}

