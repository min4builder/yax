#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <yax/errorcodes.h>
#include "arch.h"
#include "boot.h"
#include "conn.h"
#include "exec.h"
#include "int.h"
#include "iofs.h"
#include "libk.h"
#include "multitask.h"
#include "name.h"
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
	if(n >= 0x20 && n < 0x30) {
		iofsinterrupt(n - 0x20);
	} else if(n == 8) {
		dumpregs(r);
		printk("Double fault!\n");
		printk("IDK what to do! PANIC!!!!\n");
		halt();
	}
	if(n == 13) {
		dumpregs(r);
		halt();
	}
	(void) err;
}

void kernel_main(MultibootInfo *mbinfo)
{
	unsigned int i;
	int err;
	void *code;
	Module mod;
	initprintk();
	if((mbinfo->flags & 0x48) != 0x48 || mbinfo->modn != 1) {
		/* initrd module missing */
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

	if((err = execmod(VIRT(mod.start), mod.end - mod.start, &code, "argv", "envp")) < 0 && err > -MAXERR) {
		printk("exec = E");
		uxprintk(-err);
		cprintk('\n');
		halt();
	}
	/* exec on kernelland only changes user mappings, we are still running
	 * here until we change to user mode */
	/* no need for these anymore */
	for(i = (unsigned int) mod.start / PGLEN; i <= (unsigned int) mod.end / PGLEN; i++)
		ppgunref(i);
	vfsmount("/", iofsnew(), 0); /* TODO check for errors and stuff */
	usermode(code, (void *) err);
}

