#ifndef _STAT_H
#define _STAT_H

#include <stdint.h>
#include <sys/types.h>
#include <yax/stat.h>
#include "arch.h"
#include "macro.h"

size_t convD2M(Dir *, char *, size_t);
size_t convM2D(char *, size_t, Dir *, char *);

#define GBITPTR(b) MACRO_TOKPASTE_(GBIT, PTRLEN)(b)

#define PBITPTR(b, v) MACRO_TOKPASTE_(PBIT, PTRLEN)((b), (MACRO_TOKPASTE_(uint, MACRO_TOKPASTE_(PTRLEN, _t))) (v))

#endif /* _STAT_H */

