#ifndef _PRINTK_H
#define _PRINTK_H

void initprintk(void);

void printk(const char *s);
void nprintk(int n, const char *s);
void iprintk(int n);
void uiprintk(unsigned int n);
void xprintk(int n);
void uxprintk(unsigned int n);
void cprintk(char c);

#endif /* _PRINTK_H */

