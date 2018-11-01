#include <_yalc.h>

#if !defined(_PORT_H) && _YALC_NEED_YAX
#define _PORT_H

_YALCDEFTYPE(size_t);

void outb(size_t port, unsigned char byte);
void outw(size_t port, unsigned short word);
void outd(size_t port, unsigned int dword);
unsigned char inb(size_t port);
unsigned short inw(size_t port);
unsigned int ind(size_t port);
void insb(size_t port, size_t len, void *buf);
void insw(size_t port, size_t len, void *buf);
void insd(size_t port, size_t len, void *buf);
void ioperm(size_t, size_t, int);

#endif /* _PORT_H */

