#include <stdint.h>
#include "arch.h"
#include "printk.h"
#include "boot.h"
#include "physmman.h"

/* this is just stupid; we need a better system */
static uint8_t pgs[PGCNT];
static uint32_t usable[PGCNT / 32];
static uint32_t reserved[PGCNT / 32];
static unsigned int firstfree;
static unsigned int lastusable;

static int isusable(unsigned int pg)
{
	return usable[pg >> 5] & (1 << (pg & 0x1F)) && !(reserved[pg >> 5] & (1 << (pg & 0x1F)));
}

static void debugdump(void)
{
/*
	unsigned int c = 0;
	char newc, lastc = ' ';
	unsigned int i;
	for(i = 0; i <= lastusable; i++) {
		newc = !isusable(i) ? 'X' : pgs[i] ? '#' : '.';
		if(i == firstfree)
			newc = ':';
		if(newc == lastc) {
			c++;
		} else {
			if(c != 0) {
				cprintk('<');
				iprintk(c);
				cprintk('>');
				c = 0;
			}
			if(lastc != ' ')
				cprintk(lastc);
			lastc = newc;
		}
	}
	if(c != 0) {
		cprintk('<');
		iprintk(c);
		cprintk('>');
		c = 0;
	}
	if(lastc != ' ')
		cprintk(lastc);
	lastc = newc;
	cprintk('\n');
// */
}

void ppginit(int mml, MemoryMap *mm_)
{
	unsigned int i;
	uint8_t *mmb = (void *)mm_;
	while(mmb < (uint8_t *) mm_ + mml) {
		MemoryMap *mm = (void *)mmb;
		if(mm->type == 1) {
			for(i = mm->base / PGLEN; i < (mm->base + mm->len) / PGLEN + 1; i++) {
				usable[i >> 5] |= (1 << (i & 0x1F));
/*
				printk("can use 0x");
				xprintk(i);
				cprintk('\n');
// */
			}
		} else {
			for(i = mm->base / PGLEN; i < (mm->base + mm->len) / PGLEN + 1; i++) {
				reserved[i >> 5] |= (1 << (i & 0x1F));
/*
				printk("can't use 0x");
				xprintk(i);
				cprintk('\n');
// */
			}
		}
		mmb += mm->size + sizeof(mm->size);
	}
	for(i = KERNEL_LOAD / PGLEN; i < KERNEL_LOAD_END / PGLEN; i++) {
		reserved[i >> 5] |= (1 << (i & 0x1F));
/*
		printk("kernel 0x");
		xprintk(i);
		cprintk('\n');
// */
	}
	for(i = 0; i < PGCNT; i++) {
		if(isusable(i))
			lastusable = i;
	}
	debugdump();
}

unsigned int ppgalloc(void)
{
	unsigned int i;
	unsigned int pg = firstfree;
	pgs[pg]++;
	for(i = firstfree; i < 1 << 20; i++) {
		if(pgs[i] == 0 && isusable(i)) {
			firstfree = i;
			break;
		}
	}
	debugdump();
	return pg;
}

void ppgref(unsigned int pg)
{
/*	if(pgs[pg] == 0 || pgs[pg] == 0xFF)
		error;*/
	pgs[pg]++;
	debugdump();
}

void ppgunref(unsigned int pg)
{
/*	if(pgs[pg] == 0)
		error;*/
	pgs[pg]--;
	if(pgs[pg] == 0 && firstfree > pg && isusable(pg))
		firstfree = pg;
	debugdump();
}

void ppgreserve(unsigned int pg)
{
/*	if(pgs[pg] != 0 || isusable(pg))
		error;*/
	pgs[pg]++;
	debugdump();
}

