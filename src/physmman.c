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
static unsigned int lastusable;

static int isusable(uintptr_t pg)
{
	return usable[(pg / PGLEN) >> 5] & (1 << ((pg / PGLEN) & 0x1F)) && !(reserved[(pg / PGLEN) >> 5] & (1 << ((pg / PGLEN) & 0x1F)));
}

static void showregions(void)
{
	uintptr_t i, start;
	while(i) {
		while(i && !isusable(i))
			i += PGLEN;
		start = i;
		while(i && isusable(i))
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
	unsigned int i;
	uint8_t *mmb = (void *)mm_;
	while(mmb < (uint8_t *) mm_ + mml) {
		MemoryMap *mm = (void *)mmb;
		printk("rgn from 0x");
		uxprintk(mm->base);
		printk(" to 0x");
		uxprintk(mm->base + mm->len);
		if(mm->type == 1) {
			printk(" usable\n");
			for(i = mm->base / PGLEN; i < (mm->base + mm->len) / PGLEN + 1; i++)
				usable[i >> 5] |= (1 << (i & 0x1F));
		} else {
			printk(" type ");
			iprintk(mm->type);
			cprintk('\n');
			for(i = mm->base / PGLEN; i < (mm->base + mm->len) / PGLEN + 1; i++)
				reserved[i >> 5] |= (1 << (i & 0x1F));
		}
		mmb += mm->size + sizeof(mm->size);
	}
	printk("kernel from 0x");
	uxprintk(KERNEL_LOAD);
	printk(" to 0x");
	uxprintk(KERNEL_LOAD_END);
	cprintk('\n');
	for(i = KERNEL_LOAD / PGLEN; i < KERNEL_LOAD_END / PGLEN; i++)
		reserved[i >> 5] |= (1 << (i & 0x1F));
	for(i = 0; i < PGCNT; i++) {
		if(isusable(i * PGLEN)) {
			firstfree = i * PGLEN;
			break;
		}
	}
	for(i = 0; i < PGCNT; i++) {
		if(isusable(i * PGLEN))
			lastusable = i;
	}
	showregions();
}

uintptr_t ppgalloc(void)
{
	static int time = 0;
	uintptr_t i;
	uintptr_t pg = firstfree;
	time++;
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
/*	if(pgs[pg] == 0 || pgs[pg] == 0xFF)
		error;*/
	pgs[pg / PGLEN]++;
}

void ppgunref(uintptr_t pg)
{
/*	if(pgs[pg] == 0)
		error;*/
	pgs[pg / PGLEN]--;
	if(pgs[pg / PGLEN] == 0 && firstfree > pg && isusable(pg))
		firstfree = pg;
}

void ppgreserve(uintptr_t pg)
{
/*	if(pgs[pg] != 0 || isusable(pg))
		error;*/
	pgs[pg / PGLEN]++;
}

