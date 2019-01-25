#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

long long strtoll(char const *__restrict s, char **__restrict end, int base)
{
	int sign = 1;
	long long res = 0;
	char const *const orig = s;
	char const *afterspace;

	if(base < 0 || base > 36) {
		errno = EINVAL;
		return 0;
	}

	while(*s && isspace(*s)) s++;

	afterspace = s;

	if(*s == '+') {
		s++;
	} else if(*s == '-') {
		s++;
		sign = -1;
	}

	if(base == 0) {
		if(*s == '0' && (s[1] == 'x' || s[1] == 'X'))
			base = 16;
		else if(*s == '0')
			base = 8;
		else
			base = 10;
	}

	if(base == 16 && *s == '0' && (s[1] == 'x' || s[1] == 'X'))
		s += 2;

	while(*s) {
		char c = tolower(*s);
		int n;

		if(c >= '0' && c - '0' < base)
			n = c - '0';
		else if(c >= 'a' && c - 'a' + 10 < base)
			n = c - 'a' + 10;
		else break;

		if(res > LLONG_MAX / base) {
			/* overflow on multiplication */
			errno = ERANGE;
			if(end) *end = (char *) s;
			return sign < 0 ? LLONG_MIN : LLONG_MAX;
		}

		res *= base;

		if(res > LLONG_MAX - n) {
			/* overflow on addition */
			errno = ERANGE;
			if(end) *end = (char *) s;
			return sign < 0 ? LLONG_MIN : LLONG_MAX;
		}

		res += n;

		s++;
	}

	if(end) {
		if(s == afterspace)
			*end = (char *) orig;
		else
			*end = (char *) s;
	}

	return sign * res;
}

