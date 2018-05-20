#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <yax/errorcodes.h>
#include "arch.h"
#include "boot.h"
#include "conn.h"
#include "exec.h"
#include "int.h"
#include "libk.h"
#include "multitask.h"
#include "pic.h"
#include "pipe.h"
#include "pit.h"
#include "physmman.h"
#include "printk.h"
#include "syscall.h"
#include "sysentry.h"
#include "virtmman.h"

void dumpregs(Regs *);

void dumpregs(Regs *r)
{
	printk("eax=0x");
	uxprintk(r->eax);
	printk(" ebx=0x");
	uxprintk(r->ebx);
	printk(" ecx=0x");
	uxprintk(r->ecx);
	printk(" edx=0x");
	uxprintk(r->edx);
	printk(" esi=0x");
	uxprintk(r->esi);
	printk(" edi=0x");
	uxprintk(r->edi);
	printk(" ebp=0x");
	uxprintk(r->ebp);
	printk(" esp=0x");
	uxprintk(r->esp);
	cprintk('\n');
}

void int_handler(Regs *r, uint8_t n, uint32_t err)
{
	printk("Interrupt ");
	iprintk(n);
	printk(" at ");
	uxprintk(r->cs);
	cprintk(':');
	uxprintk(r->eip);
	cprintk('\n');
	if(n >= 0x20 && n < 0x30) {
		piceoi(n - 0x20);
	} else if(n == 8) {
		printk("Double fault!\n");
		printk("IDK what to do! PANIC!!!!\n");
		halt();
	}
	if(n == 13)
		halt();
	(void) err;
}

void page_fault(Regs *r, void *addr, uint32_t err)
{
	dumpregs(r);
	printk("Page fault");
	if(!(err & 0x1))
		printk("-not present");
	if(err & 0x2)
		printk("-on write");
	if(!(err & 0x4))
		printk("-by kernel");
	else {
		printk("-by process ");
		iprintk(curproc->pid);
	}
	if(err & 0x8)
		printk("-reserved bit set");
	if(err & 0x10)
		printk("-on execute");
	printk("-at 0x");
	uxprintk(r->cs);
	cprintk(':');
	uxprintk(r->eip);
	if(addr < (void *) 4096)
		printk("-null pointer dereference");
	printk("-on access 0x");
	uxprintk((uint32_t)addr);
	cprintk('\n');
	printk("PDE 0x");
	uxprintk((uint32_t) &(*PGDIR)[(unsigned int) addr / PGLEN / 1024]);
	printk(" PT 0x");
	uxprintk((uint32_t) &(*PT((unsigned int) addr / PGLEN / 1024))[(unsigned int) addr / PGLEN % 1024]);
	if((*PGDIR)[(unsigned int) addr / PGLEN / 1024] & PGPRESENT) {
		printk("PDE present");
		if((*PT((unsigned int) addr / PGLEN / 1024))[(unsigned int) addr / PGLEN % 1024] & PGPRESENT)
			printk("-PTE present");
		cprintk('\n');
	}
	halt();
}

void kernel_main(MultibootInfo *mbinfo)
{
	unsigned int i;
	int err;
	void *code;
	Module mod;
	initprintk();
	if((mbinfo->flags & 0x48) != 0x48 || mbinfo->modn != 1) {
		printk("Invalid boot\n");
		return;
	}
	mod = mbinfo->mods[0];
	ppginit(mbinfo->mmaplen, mbinfo->mmap);
	/* do this before anyone uses these pages accidentally */
	for(i = (unsigned int) mod.start / PGLEN; i <= (unsigned int) mod.end / PGLEN; i++)
		ppgreserve(i);
	clearkidmap();
	procsetup();
	picinit();
	pitinit();
	inton();


	if(procrfork(RFPROC|RFMEM|RFFDG) != 0)
		for(;;) idle();

	if((err = exec(VIRT(mod.start), &code, "argv", "envp")) < 0 && err > -MAXERR) {
		printk("exec = E");
		uxprintk(-err);
		cprintk('\n');
		halt();
	}
	/* no need for these anymore */
	for(i = (unsigned int) mod.start / PGLEN; i <= (unsigned int) mod.end / PGLEN; i++)
		ppgunref(i);
	usermode(code, (void *) err);
}

