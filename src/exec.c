#include <stdint.h>
#include <yax/errorcodes.h>
#include "arch.h"
#include "boot.h"
#include "libk.h"
#include "pgdir.h"
#include "printk.h"
#include "virtmman.h"
#include "exec.h"

static uint16_t wordle(uint8_t *p)
{
	return p[0] + (p[1] << 8);
}

static uint32_t dwordle(uint8_t *p)
{
	return p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
}

uint32_t exec(uint8_t *mod, void **entryp, char *argv, char *envp)
{
	uint32_t phs/*, shs, str*/;
	uint8_t *ph/*, *sh*/, *eph/*, *esh*/;
	PgDir *pd;
	uint8_t *stack, *sp, *argvp, *envpp;
	size_t arlen, envlen;
	if(memcmp(mod, "\x7f""ELF", 4) != 0)
		return -ENOEXEC;
	if(mod[5] != 1)
		return -ENOEXEC;
	if(mod[6] != 1)
		return -ENOEXEC;
	if(mod[7] != 0)
		return -ENOEXEC;
	if(wordle(&mod[16]) != 2)
		return -ENOEXEC;
	if(wordle(&mod[18]) != 3)
		return -ENOEXEC;
	if(dwordle(&mod[20]) != 1)
		return -ENOEXEC;
	*entryp = (void *) dwordle(&mod[24]);
	ph = mod + dwordle(&mod[28]);
/*	sh = mod + dwordle(&mod[32]);
*/
	if(dwordle(&mod[36]) != 0)
		return -ENOEXEC;
	if(wordle(&mod[40]) < 52)
		return -ENOEXEC;
	phs = wordle(&mod[42]);
	eph = ph + phs * wordle(&mod[44]);
/*	shs = wordle(&mod[46]);
	esh = sh + shs * wordle(&mod[48]);
	str = wordle(&mod[50]);
*/
	pd = vpgnew();
	pd = switchpgdir(pd);
	while(ph < eph) {
		switch(dwordle(&ph[0])) {
		case 1: {
			uint32_t addr, size, lsize;
			int rprot, prot = 0;
			addr = dwordle(&ph[8]);
			size = dwordle(&ph[20]);
			lsize = dwordle(&ph[16]);
			rprot = dwordle(&ph[24]);
			if(rprot & 1)
				prot |= PROT_EXEC;
			if(rprot & 2)
				prot |= PROT_WRITE;
			if(rprot & 4)
				prot |= PROT_READ;
			if(lsize > size || addr / PGLEN == 0 || (uint8_t *) addr + size > VIRT(0))
				goto noexec;
			vpgmap((void *) (PGLEN * (addr / PGLEN)), size, prot | PROT_USER, MAP_ANONYMOUS | MAP_FIXED, 0);
			memcpy((void *) addr, mod + dwordle(&ph[4]), lsize);
			memset((uint8_t *) addr + lsize, 0, size - lsize);
			break;
		}
		case 0:
		case 4:
			break;
		default:
			goto noexec;
		}
		ph += phs;
	}
	arlen = strlen(argv) + 1;
	envlen = strlen(envp) + 1;
	stack = vpgmap(VIRT(0) - PGLEN, PGLEN, PROT_READ | PROT_WRITE | PROT_USER, MAP_ANONYMOUS | MAP_FIXED | MAP_NOSHARE, 0);
	sp = stack + PGLEN;
	argvp = STACKPUSH(sp, argv, arlen);
	envpp = STACKPUSH(sp, envp, envlen);
	STACKPUSH(sp, &argvp, sizeof(uint8_t *));
	STACKPUSH(sp, &envpp, sizeof(uint8_t *));
	pd = switchpgdir(pd);
	vpgclear();
	pd = switchpgdir(pd);
	vpgdel(pd);
	return (uint32_t) sp;
noexec:
	vpgclear();
	pd = switchpgdir(pd);
	vpgdel(pd);
	return -ENOEXEC;
}

