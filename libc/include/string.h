#ifndef _STRING_H
#define _STRING_H

#include <sys/types.h>

int memcmp(const void *, const void *, size_t);
void *memcpy(void *, const void *, size_t);
int strcmp(const char *, const char *);
size_t strlen(const char *);
char *strcpy(char *, const char *);
size_t strlcpy(char *, const char *, size_t);
char *strcat(char *, const char *);
char *strncat(char *, const char *, size_t);

#endif /* _STRING_H */

