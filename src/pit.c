#include "arch.h"
#include "pit.h"
#include "port.h"

void pitinit(void)
{
	outb(0x43, 0x34);
	outb(0x40, (1193182 / TICK) & 0xff);
	outb(0x40, ((1193182 / TICK) >> 8) & 0xff);
}

