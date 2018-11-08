#ifndef CODAS_REF_H_
#define CODAS_REF_H_

typedef struct RefCounted RefCounted;
struct RefCounted {
	_Atomic int refcnt;
	void (*free)(const RefCounted *);
};

void mkref(RefCounted *, void (*free)(const RefCounted *));
void ref(const RefCounted *);
void unref(const RefCounted *);

/* I wish we had the plan 9 struct extension */
#define mkref(x, y) mkref((RefCounted *) (x), (y))
#define ref(x) ref((const RefCounted *) (x))
#define unref(x) unref((const RefCounted *) (x))

#endif /* CODAS_REF_H_ */

