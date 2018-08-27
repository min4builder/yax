#include "lock.h"
#include "ref.h"

/* names in parentheses because of the macros of same name (see ref.h) */
void (mkref)(RefCounted *rc, void (*free)(const RefCounted *))
{
	rc->refcnt = 1;
	rc->l = 0;
	rc->free = free;
}

void (ref)(const RefCounted *rc)
{
	ATOMIC(&((RefCounted *) rc)->l)
		((RefCounted *) rc)->refcnt++;
}

void (unref)(const RefCounted *rc)
{
	ATOMIC(&((RefCounted *) rc)->l) {
		((RefCounted *) rc)->refcnt--;
		if(((RefCounted *)rc)->refcnt <= 0)
			rc->free(rc);
	}
}

