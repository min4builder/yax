#define __YAX__
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static size_t qlen(const char *s)
{
	size_t len = 2; /* for the delimiters */
	while(*s) {
		if(*s == '\'')
			len++;
		len++, s++;
	}
	return len;
}

static char *quoteall(char *const a[])
{
	size_t slen = 0;
	char *s = 0;
	const char *i;
	while(*a) {
		i = *a;
		s = realloc(s, slen + qlen(i) + 2);
		s[slen++] = ' ';
		s[slen++] = '\'';
		while(*i) {
			if(*i == '\'')
				s[slen++] = *i;
			s[slen++] = *i;
			i++;
		}
		s[slen++] = '\'';
		s[slen] = '\0';
		a++;
	}
	if(s == 0) {
		/* array was empty */
		s = malloc(1);
		*s = '\0';
	}
	return s;
}

int execve(const char *name, char *const argv[], char *const envp[])
{
	char *nargv = quoteall(argv);
	char *nenvp = quoteall(envp);
	int err = exec(name, nargv, nenvp);
	free(nargv);
	free(nenvp);
	return err;
}

