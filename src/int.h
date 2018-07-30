#ifndef _INT_H
#define _INT_H

#include <stdint.h>
#include "arch.h"

void inton(void);
int intoff(void);
/* defined in main.c */
void int_handler(Regs *, uint8_t irq, uint32_t code);
/* defined in multitask.c */
void timer_handler(void);
/* defined in virtmman.c */
void page_fault(Regs *, void *addr, uint32_t code);

#endif /* _INT_H */

