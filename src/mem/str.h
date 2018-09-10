#ifndef _MEM_STR_H
#define _MEM_STR_H

#include "mem/ref.h"

typedef struct {
	RefCounted refcnt;
	const char *s;
} Str;

Str *strmk(char *);
Str *strcopy(Str *);

#endif /* _MEM_STR_H */

