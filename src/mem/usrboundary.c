#include "mem/malloc.h"
#include "mem/pgdir.h"
#include "mem/phys.h"
#include "mem/user.h"
#include "mem/usrboundary.h"
#include "mem/ref.h"

static int verusrpage(uintptr_t p, enum mapprot prot)
{
	PgDir *pt;
	if(!((*PGDIR)[(p / PGLEN) / 1024] & PGPRESENT))
		return 0;
	pt = PT((p / PGLEN) / 1024);
	if(!((*pt)[(p / PGLEN) % 1024] & PGPRESENT))
		return 0;
	if(!((*pt)[(p / PGLEN) % 1024] & PGUSER))
		return 0;
	if(prot & PROT_WRITE && !((*pt)[(p / PGLEN) % 1024] & PGWRITEABLE))
		return 0;
	return 1;
}

int verusrptr(const void *p, size_t len, enum mapprot prot)
{
	uintptr_t pgs = (uintptr_t) p;
	uintptr_t i;
	for(i = pgs; i < pgs + len; i += PGLEN) {
		if(!verusrpage(i, prot))
			return 0;
	}
	return 1;
}

int verusrstr(const char *s, enum mapprot prot)
{
	/* TODO this is overly inneficient */
	do {
		if(!verusrpage((uintptr_t) s, prot))
			return 0;
	} while(*s++ != '\0');
	return 1;
}

static void freeup(const RefCounted *rc)
{
	PgList *pl = (PgList *) rc;
	unsigned int i;
	for(i = 0; i < pl->len; i++) {
		Page *p = pmapget(pl->e[i]);
		unref(p);
	}
	free(pl->e);
	free(pl);
}

PgList *getusrptr(const void *p, size_t len)
{
	uintptr_t pgs = (uintptr_t) p;
	uintptr_t i;
	PgDir *pt;
	PgList *pl = calloc(1, sizeof(PgList));
	pl->e = calloc((len + PGLEN - 1) / PGLEN, sizeof(PgEntry));
	mkref(pl, freeup);
	pl->len = len;
	pl->delta = pgs % PGLEN;
	for(i = pgs; i < pgs + len; i += PGLEN) {
		Page *p;
		/* TODO security */
		pt = PT((i / PGLEN) / 1024);
		pl->e[(i - pgs) / PGLEN] = (*pt)[(i / PGLEN) % 1024] & PGADDR;
		p = pmapget(pl->e[(i - pgs) / PGLEN]);
		if(p)
			ref(p);
	}
	return pl;
}

void *putusrptr(PgList *pl)
{
	unsigned int plen = pl->len + PGLEN - pl->len % PGLEN;
	uintptr_t pgs = pgfind(plen, 1);
	uintptr_t i;
	if(pgs == 0)
		return 0;
	ref(pl);
	for(i = pgs; i < pgs + pl->len; i += PGLEN) {
		/* TODO security */
		pgmap(i, pgemk(pl->e[(i - pgs) / PGLEN], PROT_READ | PROT_WRITE, 1));
	}
	return (void *) (pgs + pl->delta);
}

void freeusrptr(PgList *pl, void *ptr)
{
	uintptr_t pgs = (uintptr_t) ptr;
	uintptr_t i;
	/* pages themselves are referenced by the PgList, so they die with it */
	for(i = pgs; i < pgs + pl->len; i += PGLEN)
		pgunmap(i);
	unref(pl);
}

