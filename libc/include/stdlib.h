#include <_yalc.h>

#ifndef _STDLIB_H_ANSI
#define _STDLIB_H_ANSI

_YALCDEFTYPE(size_t);
_YALCDEFTYPE(wchar_t);

void abort(void);
void *malloc(size_t);
void *realloc(void *, size_t);
void free(void *);

#endif /* _STDLIB_H_ANSI */

#if !defined(_STDLIB_H_YAX) && _YALC_NEED_YAX
#define _STDLIB_H_YAX

char *itoa(int, char *, size_t);

#endif /* _STDLIB_H_YAX */

