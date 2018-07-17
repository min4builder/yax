#ifndef _LIBK_H
#define _LIBK_H

#include <stddef.h>

void *memset(void *, int, size_t);
void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);
size_t strlen(const char *);
size_t strlcpy(char *, const char *, size_t);

#endif /* _LIBK_H */

