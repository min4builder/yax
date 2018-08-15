#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/types.h>

void abort(void);
void *malloc(size_t);
void *realloc(void *, size_t);
void free(void *);

#endif /* _STDLIB_H */

