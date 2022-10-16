#ifndef CODAS_VECTOR_H_
#define CODAS_VECTOR_H_

#include <stddef.h>
#include <codas/generic.h>

struct Vector {
	void *ptr;
	size_t cap, len;
	Typesz type;
	void (*drop)(void *);
};

/* Unfortunately there is no type safety here; this is for annotation purposes
   only. */
#define Vec(T) struct Vector

/* Vec(T) vecnew(type T, void (*drop)(void *)) */
struct Vector vecnew_(Typesz, void (*)(void *));
#define vecnew(T, drop) vecnew_(sizeof(T), drop)

/* Vec(T) vecwithcap(type T, void (*drop)(void *), size_t cap) */
struct Vector vecwithcap_(Typesz, void (*)(void *), size_t);
#define vecwithcap(T, drop, cap) vecwith_cap_(sizeof(T), drop, cap)

/* void vecpush(Vec(T) *vec, T lval elem) */
void vecpush_(struct Vector *, void const *);
#define vecpush(v, e) vecpush_(v, &(e))

static inline void const *vecget(struct Vector const *vec, size_t n)
{
	return (char const *) vec->ptr + n * vec->type;
}

/* void vecset(Vec(T) *vec, size_t n, T lval elem) */
static inline void vecset_(struct Vector *, size_t, void const *);
#define vecset(v, n, e) vecset_(v, n, &(e))

static inline void vecset_(struct Vector *vec, size_t n, void const *elem)
{
	extern void *memcpy(void *restrict, void const *restrict, size_t);
	memcpy((char *) vec->ptr + n * vec->type, elem, vec->type);
}

static inline void vecremswap(struct Vector *vec, size_t n)
{
	extern void *memcpy(void *restrict, void const *restrict, size_t);
	memcpy((char *) vec->ptr + n * vec->type, (char const *) vec->ptr + (vec->len - 1) * vec->type, vec->type);
	vec->len--;
}

#define veccap(v) ((v)->cap)
#define veclen(v) ((v)->len)

static inline void const *veciter(struct Vector *vec)
{
	if(vec->len == 0)
		return 0;
	else
		return vec->ptr;
}

static inline void const *veciternext(struct Vector *vec, void const *cur)
{
	if((cur - vec->ptr) / vec->type >= vec->len)
		return 0;
	else
		return (char const *) cur + vec->type;
}

void vecdrop(struct Vector *);

#endif /* CODAS_VECTOR_H_ */

