#include <_yalc.h>

#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <yax/mapflags.h>

_YALCDEFTYPE(mode_t);
_YALCDEFTYPE(size_t);
_YALCDEFTYPE(off_t);

void *mmap(void *, size_t, int, int, int, off_t);
int munmap(void *, size_t);

#endif /* _SYS_MMAN_H */

