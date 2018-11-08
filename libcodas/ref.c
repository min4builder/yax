#include <sys/lock.h>
#include <codas/ref.h>

/* names in parentheses because of the macros of same name (see ref.h) */
void (mkref)(RefCounted *rc, void (*free)(const RefCounted *))
{
	rc->refcnt = 1;
	rc->free = free;
}

void (ref)(const RefCounted *rc)
{
	((RefCounted *) rc)->refcnt++;
}

void (unref)(const RefCounted *rc)
{
	
	if(!--((RefCounted *) rc)->refcnt)
		rc->free(rc);
	else if(rc->refcnt < 0)
		{(void) *(volatile int *)0;}
}

