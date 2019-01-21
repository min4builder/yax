#include <_yalc.h>

#ifndef _STDLIB_H_ANSI
#define _STDLIB_H_ANSI

_YALCDEFTYPE(size_t);
_YALCDEFTYPE(wchar_t);

void abort(void);
int abs(int);
_Noreturn void exit(int);
void free(void *);
void *malloc(size_t);
void *realloc(void *, size_t);
long strtol(char const *__restrict, char **__restrict, int);
long long strtoll(char const *__restrict, char **__restrict, int);

#endif /* _STDLIB_H_ANSI */

#if !defined(_STDLIB_H_YAX) && _YALC_NEED_YAX
#define _STDLIB_H_YAX

char *itoa(int, char *, size_t);

#endif /* _STDLIB_H_YAX */

