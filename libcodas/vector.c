#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <codas/generic.h>
#include <codas/vector.h>

struct Vector vecnew_(Typesz type, void (*drop)(void *))
{
	struct Vector vec;
	vec.ptr = 0;
	vec.cap = 0;
	vec.len = 0;
	vec.type = type;
	vec.drop = drop;
	return vec;
}

struct Vector vecwithcap_(Typesz type, void (*drop)(void *), size_t cap)
{
	struct Vector vec;
	vec.ptr = malloc(cap * type);
	vec.cap = cap;
	vec.len = 0;
	vec.type = type;
	vec.drop = drop;
	return vec;
}

static void grow(struct Vector *vec)
{
	vec->ptr = realloc(vec->ptr, (vec->cap + 16) * vec->type);
	vec->cap += 16;
}

void vecpush_(struct Vector *vec, void const *elem)
{
	if(vec->cap == vec->len)
		grow(vec);
	memcpy((char *) vec->ptr + vec->len * vec->type, elem, vec->type);
	vec->len++;
}

void vecdrop(struct Vector *vec)
{
	size_t i;
	if(vec->drop) {
		for(i = 0; i < vec->len; i++)
			vec->drop((char *) vec->ptr + i * vec->type);
	}
	if(vec->cap > 0)
		free(vec->ptr);
}

