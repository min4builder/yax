#ifndef _PHYSMMAN_H
#define _PHYSMMAN_H

#include <sys/types.h>
#include "boot.h"

void ppginit(int, MemoryMap *);
uintptr_t ppgalloc(void);
void ppgref(uintptr_t);
void ppgunref(uintptr_t);
void ppgreserve(uintptr_t);

#endif /* _PHYSMMAN_H */

