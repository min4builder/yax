#ifndef _PORT_H
#define _PORT_H

#include <stdint.h>

void outb(int port, uint8_t byte);
uint8_t inb(int port);
void iowait(void);

#endif /* _PORT_H */

