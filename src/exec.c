#include <stdint.h>
#include <sys/types.h>
#include <yax/bit.h>
#include <yax/errorcodes.h>
#include "arch.h"
#include "boot.h"
#include "libk.h"
#include "multitask.h"
#include "pgdir.h"
#include "printk.h"
#include "virtmman.h"
#include "exec.h"

static int elfident(uint8_t *buf, size_t len)
{
	if(len < 4 || memcmp(buf, "\x7f""ELF", 4) != 0)
		return 0;
	if(len >= 5 && buf[4] != 1) /* 32-bit */
		return 0;
	if(len >= 6 && buf[5] != 1) /* LE */
		return 0;
	if(len >= 7 && buf[6] != 1) /* ELF version */
		return 0;
	if(len >= 8 && buf[7] != 0) /* SYSV ABI */
		return 0;
	if(len >= 18 && GBIT16(&buf[16]) != 2) /* executable */
		return 0;
	if(len >= 20 && GBIT16(&buf[18]) != 3) /* x86 */
		return 0;
	if(len >= 24 && GBIT32(&buf[20]) != 1) /* ELF version again */
		return 0;
	if(len >= 40 && GBIT32(&buf[36]) != 0) /* flags (none are defined) */
		return 0;
	if(len >= 44 && GBIT32(&buf[40]) < 52) /* header size */
		return 0;
	return 1;
}

static uint32_t elfload(uint8_t *buf, void **entryp, char *argv, char *envp)
{
	uint32_t phs;
	uint8_t *ph, *eph;
	PgDir *pd;
	uint8_t *stack, *sp, *argvp, *envpp;
	size_t arlen, envlen;
	*entryp = (void *) GBIT32(&buf[24]);
	ph = buf + GBIT32(&buf[28]);
	phs = GBIT16(&buf[42]);
	eph = ph + phs * GBIT16(&buf[44]);
	pd = vpgnew();
	pd = switchpgdir(pd);
	while(ph < eph) {
		switch(GBIT32(&ph[0])) {
		case 1: {
			uint32_t addr, size, lsize;
			int rprot, prot = 0;
			addr = GBIT32(&ph[8]);
			size = GBIT32(&ph[20]);
			lsize = GBIT32(&ph[16]);
			rprot = GBIT32(&ph[24]);
			if(rprot & 1)
				prot |= PROT_EXEC;
			if(rprot & 2)
				prot |= PROT_WRITE;
			if(rprot & 4)
				prot |= PROT_READ;
			if(lsize > size || addr / PGLEN == 0 || (uint8_t *) addr + size > VIRT(0))
				goto noexec;
			vpgmap((void *) addr, size, prot | PROT_USER, MAP_ANONYMOUS | MAP_FIXED, 0, 0, 0);
			memcpy((void *) addr, buf + GBIT32(&ph[4]), lsize);
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
	stack = vpgmap(VIRT(0) - PGLEN, PGLEN, PROT_READ | PROT_WRITE | PROT_USER, MAP_ANONYMOUS | MAP_FIXED | MAP_NOSHARE, 0, 0, 0);
	sp = stack + PGLEN;
	envpp = STACKPUSH(sp, envp, envlen);
	argvp = STACKPUSH(sp, argv, arlen);
	STACKPUSH(sp, &envpp, sizeof(uint8_t *));
	STACKPUSH(sp, &argvp, sizeof(uint8_t *));
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

uint32_t execmod(void *mod, size_t len, void **entryp, char *argv, char *envp)
{
	if(elfident(mod, len))
		return elfload(mod, entryp, argv, envp);
	return -ENOEXEC;
}

static uint32_t elfrun(Conn *c, void **entryp, char *argv, char *envp)
{
#define epread(c, buf, len, off) do { int err; if((err = connpread(c, buf, len, off)) < (len)) return err < 0 ? err : -EIO; } while(0)
	uint32_t phs;
	size_t ph, eph;
	PgDir *pd;
	uint8_t *stack, *sp, *argvp, *envpp;
	size_t arlen, envlen;
	char buf[4];
	epread(c, buf, 4, 24);
	*entryp = (void *) GBIT32(buf);
	epread(c, buf, 4, 28);
	ph = GBIT32(buf);
	epread(c, buf, 2, 42);
	phs = GBIT16(buf);
	epread(c, buf, 2, 44);
	eph = ph + phs * GBIT16(buf);
	pd = vpgnew();
	pd = switchpgdir(pd);
	while(ph < eph) {
		epread(c, buf, 4, ph);
		switch(GBIT32(buf)) {
		case 1: {
			uint32_t addr, size, lsize, off;
			int rprot, prot = 0;
			epread(c, buf, 4, ph + 4);
			off = GBIT32(buf);
			epread(c, buf, 4, ph + 8);
			addr = GBIT32(buf);
			epread(c, buf, 4, ph + 16);
			lsize = GBIT32(buf);
			epread(c, buf, 4, ph + 20);
			size = GBIT32(buf);
			epread(c, buf, 4, ph + 24);
			rprot = GBIT32(buf);
			if(rprot & 1)
				prot |= PROT_EXEC;
			if(rprot & 2)
				prot |= PROT_WRITE;
			if(rprot & 4)
				prot |= PROT_READ;
			if(lsize > size || addr / PGLEN == 0 || (uint8_t *) addr + size > VIRT(0))
				goto noexec;
			vpgmap((void *) addr, size, prot | PROT_USER, MAP_PRIVATE | MAP_FIXED, c, off, lsize);
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
	stack = vpgmap(VIRT(0) - PGLEN, PGLEN, PROT_READ | PROT_WRITE | PROT_USER, MAP_ANONYMOUS | MAP_FIXED | MAP_NOSHARE, 0, 0, 0);
	sp = stack + PGLEN;
	envpp = STACKPUSH(sp, envp, envlen);
	argvp = STACKPUSH(sp, argv, arlen);
	STACKPUSH(sp, &envpp, sizeof(uint8_t *));
	STACKPUSH(sp, &argvp, sizeof(uint8_t *));
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
#undef epread
}

uint32_t exec(Conn *c, void **entryp, char *argv, char *envp)
{
	char buf[32];
	ssize_t err = connpread(c, buf, 32, 0);
	if(err < 0)
		return err;
	if(elfident(buf, err)) {
		uint32_t ret;
		printk("{Exec ");
		iprintk(curproc->pid);
		printk(" = ");
		ret = elfrun(c, entryp, argv, envp);
		uxprintk(ret);
		cprintk('}');
		return ret;
	}
	return -ENOEXEC;
}

