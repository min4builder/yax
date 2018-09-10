#include <_yalc.h>

#ifndef _STRING_H_ANSI
#define _STRING_H_ANSI

_YALCDEFTYPE(size_t);

int memcmp(const void *, const void *, size_t);
void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int c, size_t);
int strcmp(const char *, const char *);
size_t strlen(const char *);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
char *strncat(char *, const char *, size_t);
int strncmp(const char *, const char *, size_t);

#endif /* _STRING_H_ANSI */

#if !defined(_STRING_H_YAX) && _YALC_NEED_YAX
#define _STRING_H_YAX

size_t strlcat(char *, const char *, size_t);
size_t strlcpy(char *, const char *, size_t);

#endif /* _STRING_H_YAX */

