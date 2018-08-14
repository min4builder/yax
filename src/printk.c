#ifdef NDEBUG
#undef NDEBUG
#endif
#include "pic.h"
#include "port.h"
#include "printk.h"

#define PORT 0x3f8

void initprintk(void)
{
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x80);
	outb(PORT + 0, 0x03);
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x03);
	outb(PORT + 2, 0xc7);
	outb(PORT + 4, 0x0b);
}

void cprintk(char c)
{
	while(!(inb(PORT + 5) & 0x20)) {}
	outb(PORT, c);
}

static void pcprintk(char c)
{
	if(c >= ' ' && c < 0x7F || c == '\n' || c == '\t')
		cprintk(c);
	else {
		cprintk('\\');
		cprintk('0' + (c >> 6));
		cprintk('0' + ((c >> 3) & 07));
		cprintk('0' + (c & 07));
	}
}

void nprintk(int n, const char *s)
{
	int i;
	for(i = 0; i < n; i++)
		pcprintk(s[i]);
}

void iprintk(int n)
{
	char num[16]; /* FIXME: should be enough */
	int cpos = 15;
	int negative = n < 0;
	if(negative)
		n = -n;
	do {
		num[cpos--] = '0' + n % 10;
		n /= 10;
	} while(cpos >= 0 && n);
	if(negative)
		num[cpos--] = '-';
	nprintk(16 - cpos - 1, num + cpos + 1);
}

void uiprintk(unsigned int n)
{
	char num[16]; /* FIXME: should be enough */
	int cpos = 15;
	do {
		num[cpos--] = '0' + n % 10;
		n /= 10;
	} while(cpos >= 0 && n);
	nprintk(16 - cpos - 1, num + cpos + 1);
}

void xprintk(int n)
{
	char num[16]; /* FIXME: should be enough */
	int cpos = 15;
	do {
		num[cpos--] = (n & 0xf) < 10 ? '0' + (n & 0xf) : 'A' + (n & 0xf) - 10;
		n >>= 4;
	} while(cpos >= 0 && n);
	nprintk(16 - cpos - 1, num + cpos + 1);
}

void uxprintk(unsigned int n)
{
	char num[16]; /* FIXME: should be enough */
	int cpos = 15;
	do {
		num[cpos--] = (n & 0xf) < 10 ? '0' + (n & 0xf) : 'A' + (n & 0xf) - 10;
		n >>= 4;
	} while(cpos >= 0 && n);
	nprintk(16 - cpos - 1, num + cpos + 1);
}

void ubprintk(unsigned int n)
{
	char num[32]; /* FIXME: should be enough */
	int cpos = 31;
	do {
		num[cpos--] = '0' + (n & 1);
		n >>= 1;
	} while(cpos >= 0 && n);
	nprintk(32 - cpos - 1, num + cpos + 1);
}

void printk(const char *s)
{
	int i;
	for(i = 0; s[i]; i++)
		pcprintk(s[i]);
}

