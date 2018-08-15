#include <string.h>
#include <sys/types.h>

size_t strlcpy(char *to, const char *from, size_t tolen)
{
	size_t fromlen = strlen(from);
	if(tolen != 0) {
		memcpy(to, from, fromlen + 1 < tolen ? fromlen + 1 : tolen - 1);
		to[tolen - 1] = '\0';
	}
	return fromlen;
}

