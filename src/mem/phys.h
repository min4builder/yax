#ifndef _MEM_PHYS_H
#define _MEM_PHYS_H

#include <sys/types.h>
#include "boot.h"

void ppginit(int, MemoryMap *);
uintptr_t ppgalloc(void);
int ppgreserve(uintptr_t);
void ppgfree(uintptr_t);

#endif /* _MEM_PHYS_H */

