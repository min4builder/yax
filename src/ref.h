#ifndef _REF_H
#define _REF_H

#include "lock.h"

typedef struct RefCounted RefCounted;
struct RefCounted {
	int refcnt;
	Lock l;
	void (*free)(const RefCounted *);
};

void mkref(RefCounted *, void (*free)(const RefCounted *));
void ref(const RefCounted *);
void unref(const RefCounted *);

/* I wish we had the plan 9 struct extension */
#define mkref(x, y) mkref((RefCounted *) (x), (y))
#define ref(x) ref((const RefCounted *) (x))
#define unref(x) unref((const RefCounted *) (x))

#endif /* _REF_H */

