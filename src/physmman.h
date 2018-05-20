#ifndef _PHYSMMAN_H
#define _PHYSMMAN_H

#include "boot.h"

void ppginit(int, MemoryMap *);
unsigned int ppgalloc(void);
void ppgref(unsigned int pg);
void ppgunref(unsigned int pg);
void ppgreserve(unsigned int pg);

#endif /* _PHYSMMAN_H */

