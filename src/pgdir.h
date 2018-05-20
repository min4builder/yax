#ifndef _PGDIR_H
#define _PGDIR_H

#include "virtmman.h"

void pginval(unsigned int pg);
void ptinval(unsigned int pt);
PgDir *getpgdir(void);
PgDir *switchpgdir(PgDir *);

#endif /* _PGDIR_H */

