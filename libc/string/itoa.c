#include <stdlib.h>

char *itoa(int n, char *s, size_t len)
{
	int top = 0;
	int n2 = n;
	while(n2 && (size_t) top < len - 1)
		n2 /= 10;
	s[top--] = '\0';
	while(n && top >= 0) {
		s[top--] = '0' + (n % 10);
		n /= 10;
	}
	return s;
}

