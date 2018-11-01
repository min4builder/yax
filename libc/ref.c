#define __YAX__
#include <sys/lock.h>
#include <sys/ref.h>

/* names in parentheses because of the macros of same name (see sys/ref.h) */
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
		if(rc->refcnt == 0)
			rc->free(rc);
		else if(rc->refcnt < 0)
			{(void) *(volatile int *)0;}
	}
}

