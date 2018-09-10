#include <_yalc.h>

#if !defined(_PORT_H) && _YALC_NEED_YAX
#define _PORT_H

void outb(int port, unsigned char byte);
unsigned char inb(int port);
void __iopl(void);

#endif /* _PORT_H */

