#include <_yalc.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STRING_H_ANSI
#define _STRING_H_ANSI

#ifndef NULL
#define NULL _YaclNULL
#endif

_YALCDEFTYPE(size_t);

int memcmp(const void *, const void *, size_t);
void *memcpy(void *__restrict, const void *__restrict, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int c, size_t);
char *strcat(char *, const char *);
int strcmp(const char *, const char *);
char *strcpy(char *__restrict, const char *__restrict);
size_t strlen(const char *);
char *strncat(char *__restrict, const char *__restrict, size_t);
int strncmp(const char *, const char *, size_t);

#endif /* _STRING_H_ANSI */

#if !defined(_STRING_H_BSD) && _YALC_NEED_BSD
#define _STRING_H_BSD

size_t strlcat(char *__restrict, const char *__restrict, size_t);
size_t strlcpy(char *__restrict, const char *__restrict, size_t);

#endif /* _STRING_H_BSD */

#ifdef __cplusplus
}
#endif

