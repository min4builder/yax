#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <sys/types.h>
#include <yax/mapflags.h>

void *mmap(void *, size_t, int, int, int, off_t);
int munmap(void *, size_t);

#endif /* _SYS_MMAN_H */

