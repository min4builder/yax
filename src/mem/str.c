#include <string.h>
#include "mem/malloc.h"
#include "mem/str.h"

static void sfree(const RefCounted *rc)
{
	Str *s = (Str *)rc;
	free((char *) s->s);
	free(s);
}

Str *strmk(char *s)
{
	Str *str = malloc(sizeof(*str));
	mkref(str, &sfree);
	str->s = s;
	return str;
}

Str *strcopy(Str *str)
{
	Str *nstr = malloc(sizeof(*nstr));
	char *s = malloc(strlen(str->s)+1);
	mkref(nstr, &sfree);
	nstr->s = s;
	strcpy(s, str->s);
	return nstr;
}

