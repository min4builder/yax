#ifndef _PRINTK_H
#define _PRINTK_H

#ifndef NDEBUG
void initprintk(void);

void printk(const char *s);
void nprintk(int n, const char *s);
void iprintk(int n);
void uiprintk(unsigned int n);
void xprintk(int n);
void uxprintk(unsigned int n);
void ubprintk(unsigned int n);
void cprintk(char c);
#else
#define initprintk() ((void) 0)
#define printk(s) ((void) 0)
#define nprintk(n, s) ((void) 0)
#define iprintk(n) ((void) 0)
#define uiprintk(n) ((void) 0)
#define xprintk(n) ((void) 0)
#define uxprintk(n) ((void) 0)
#define ubprintk(n) ((void) 0)
#define cprintk(c) ((void) 0)
#endif

#endif /* _PRINTK_H */

