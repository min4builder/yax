#include <_yalc.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STRING_H_ANSI
#define _STRING_H_ANSI

#ifndef NULL
#define NULL _YalcNULL
#endif

_YALCDEFTYPE(size_t);

void *memchr(void const *, int, size_t);
int memcmp(const void *, void const *, size_t);
void *memcpy(void *__restrict, void const *__restrict, size_t);
void *memmove(void *, void const *, size_t);
void *memset(void *, int, size_t);
char *strcat(char *, char const *);
char *strchr(char const *, int);
int strcmp(char const *, char const *);
char *strcpy(char *__restrict, const char *__restrict);
size_t strcspn(char const *, char const *);
char *strerror(int);
size_t strlen(char const *);
char *strncat(char *__restrict, char const *__restrict, size_t);
int strncmp(char const *, char const *, size_t);
char *strrchr(char const *, int);
char *strstr(char const *, char const *);

#endif /* _STRING_H_ANSI */

#if !defined(_STRING_H_POSIX) && _YALC_NEED_POSIX
#define _STRING_H_POSIX

char *strdup(char const *);
char *strndup(char const *, size_t);

#endif /* _STRING_H_POSIX */

#if !defined(_STRING_H_BSD) && _YALC_NEED_BSD
#define _STRING_H_BSD

size_t strlcat(char *__restrict, char const *__restrict, size_t);
size_t strlcpy(char *__restrict, char const *__restrict, size_t);

#endif /* _STRING_H_BSD */

#ifdef __cplusplus
}
#endif

