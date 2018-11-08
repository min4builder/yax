#ifndef _MEM_VIRTMMAN_H
#define _MEM_VIRTMMAN_H

#include <stddef.h>
#include <sys/types.h>
#include <codas/ref.h>
#include <yax/mapflags.h>
#include "conn.h"
#include "mem/pgdir.h"

/* used by physmman; must not call any of its functions */
void *vpgspecmap(uintptr_t);

void *vpgkmap(size_t, enum mapprot);
void *vpgumap(void *, size_t, enum mapprot, enum mapflags);
void *vpgpmap(void *, size_t, enum mapprot, enum mapflags, uintptr_t);
void *vpgfmap(void *, size_t, enum mapprot, enum mapflags, Conn *, off_t);
void vpgunmap(void *, size_t);

PgDir *vpgnew(void);
PgDir *vpgcopy(int);
void vpgclear(void);
void vpgdel(PgDir *);

#endif /* _MEM_VIRTMMAN_H */

