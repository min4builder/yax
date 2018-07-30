#ifndef _STRING_H
#define _STRING_H

#include <sys/types.h>

int memcmp(const void *, const void *, size_t);
void *memcpy(void *, const void *, size_t);
int strcmp(const char *, const char *);

#endif /* _STRING_H */

