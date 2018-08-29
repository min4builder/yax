#define _YAX_
#include <unistd.h>

void _exit(int n)
{
	int top = 31;
	char s[32];
	s[top--] = '\0';
	n &= 0xFF;
	while(n) {
		s[top--] = '0' + (n % 10);
		n /= 10;
	}
	_exits(&s[top+1]);
}

