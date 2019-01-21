#ifndef __YAX__
#error "Declare your use of __YAX__ before using YAX-specific files"
#endif

#ifndef YAX_PORT_H_
#define YAX_PORT_H_

#include <sys/types.h>

void outb(size_t port, unsigned char byte);
void outw(size_t port, unsigned short word);
void outd(size_t port, unsigned int dword);
unsigned char inb(size_t port);
unsigned short inw(size_t port);
unsigned int ind(size_t port);
void insb(size_t port, size_t len, void *buf);
void insw(size_t port, size_t len, void *buf);
void insd(size_t port, size_t len, void *buf);
void outsb(size_t port, size_t len, void const *buf);
void outsw(size_t port, size_t len, void const *buf);
void outsd(size_t port, size_t len, void const *buf);
void ioperm(size_t, size_t, int);

#endif /* YAX_PORT_H_ */

