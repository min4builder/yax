#include <_yalc.h>

#if !defined(_SYS_REF_H) && defined(_YALC_NEED_YAX)
#define _SYS_REF_H

#include <sys/lock.h>

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

#endif /* _SYS_REF_H */

