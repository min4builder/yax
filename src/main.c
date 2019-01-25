#define __YAX__
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <yax/errorcodes.h>
#include "arch.h"
#include "boot.h"
#include "exec.h"
#include "fs/conn.h"
#include "fs/iofs.h"
#include "fs/name.h"
#include "fs/pipe.h"
#include "int.h"
#include "mem/malloc.h"
#include "mem/phys.h"
#include "mem/virt.h"
#include "multitask.h"
#include "pic.h"
#include "pit.h"
#include "printk.h"
#include "syscall.h"
#include "sysentry.h"

void dumpregs(Regs *);

void dumpregs(Regs *r)
{
	printk("eip=0x");
	uxprintk(r->eip);
	printk(" eax=0x");
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
	printk(" cs=0x");
	uxprintk(r->cs);
	printk(" ds=0x");
	uxprintk(r->ds);
	printk(" es=0x");
	uxprintk(r->es);
	printk(" fs=0x");
	uxprintk(r->fs);
	printk(" gs=0x");
	uxprintk(r->gs);
	printk(" ss=0x");
	uxprintk(r->ss);
	cprintk('\n');
}

void int_handler(Regs *r, uint8_t n, uint32_t err)
{
	if(n >= 0x20 && n < 0x30) {
		/*iofsinterrupt(n - 0x20);*/
		return;
	}
	if(n == 8) {
		dumpregs(r);
		printk("Double fault!\n");
		printk("IDK what to do! PANIC!!!!\n");
		halt();
	} else if(n == 6) {
		dumpregs(r);
		printk("Invalid instruction\n");
		halt();
	}
	if(n == 13) {
		dumpregs(r);
		printk("General protection fault\n");
		halt();
	}
	dumpregs(r);
	printk("Exception ");
	iprintk(n);
	printk(" err=");
	uxprintk(err);
	cprintk('\n');
	halt();
}

void kernel_main(MultibootInfo *mbinfo)
{
	uintptr_t i;
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
	for(i = (uintptr_t) mod.start; i <= (uintptr_t) mod.end; i += PGLEN)
		ppgreserve(i);
	clearkidmap();
	procsetup();
	picinit();
	pitinit();
	inton();

	if(procrfork(RFPROC|RFMEM|RFFDG) != 0)
		for(;;) idle();

	/* exec on kernelland only changes user mappings, we will still be
	 * running here until we change to user mode */
	if((err = execmod(VIRT(mod.start), mod.end - mod.start, &code, "'argv'", "'envp'")) < 0 && err > -MAXERR) {
		printk("exec = E");
		uxprintk(-err);
		cprintk('\n');
		halt();
	}

	/* no need for these anymore */
	for(i = (uintptr_t) mod.start; i <= (uintptr_t) mod.end; i += PGLEN)
		ppgfree(i);
	/*vfsmount("/", iofsnew(), 0); *//* TODO check for errors and stuff */
	usermode(code, (void *) err);
}

