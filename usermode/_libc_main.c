#include <stdlib.h>
#include <unistd.h>

static char **tokenize(const char *s)
{
	int len = 1;
	char *bt, *et, *to, **a = 0;
	while(*s) {
		while(*s && (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r'))
			s++;
		if(*s) s++;
		bt = s;
		while(*s) {
			if(*s == '\'') {
				if(s[1] == '\'')
					s++;
				else
					break;
			}
			s++;
		}
		et = s;
		if(*s) s++;
		a = realloc(a, sizeof(*a) * ++len);
		to = malloc(et - bt);
		a[len - 2] = to;
		while(bt != et) {
			if(*bt == '\'')
				bt++;
			*to++ = *bt++;
		}
		*to = '\0';
	}
	a[len - 1] = 0;
	return a;
}

int main(int, char **);

void _libc_main(const char *argv, const char *envp)
{
	char **pargv = tokenize(argv);
	environ = tokenize(envp);
	int argc;
	for(argc = 0; pargv[argc]; argc++) {}
	main(argc, pargv);
}

