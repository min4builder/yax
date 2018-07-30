#ifndef _ARCH_H
#define _ARCH_H

#include <stdint.h>
#include <yax/errorcodes.h>
#include "boot.h"

#define PGCNT ((unsigned int) 1 << 20)
#define UPGCNT ((unsigned int) VIRT(0) / PGLEN / 1024)
#define PGLEN ((unsigned int) 1 << 12)

#define PGDIR ((PgDir *) 0xfffff000)
#define PT(x) ((PgDir *) (0xffc00000 + 0x1000 * (x)))
#define SPG ((unsigned int) 0xffbfd)

#define TICK 100 /*Hz*/

typedef struct {
	uint32_t edi, esi, ebp, bogus_esp, ebx, edx, ecx, eax;
	uint32_t ds, es, fs, gs;
	uint32_t eip;
	uint16_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint16_t ss;
} Regs;

#define SWITCHSYSCALLSTACK(ks) (switchsyscallstack_(ks))

#define RFORKREGADJUST(sp, cur, new) (((Regs *) sp)->ebp += (uint8_t *) (new) - (uint8_t *) (cur))

#define STACKPUSH(sp, ptr, len) memcpy(*(char **)&(sp) -= (len), (ptr), (len))
#define STACKPEEK(sp, ptr, len) ((void)memcpy((ptr), (sp), (len)))

#define PTRERR(p) ((int32_t) (p) > -MAXERR && (int32_t) (p) < 0)
#define PTR2ERR(p) ((int32_t) (p))
#define ERR2PTR(e) ((void *) (e))

#define PTRLEN 32

#endif /* _ARCH_H */

