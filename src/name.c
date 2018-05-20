#include "mux.h"
#include "name.h"

/*
typedef struct {
	char *name;
	Muxmaster *mm;
} Mount;

typedef struct {
	int refcnt;
	size_t len;
	struct Mount
} Mounttab;
*/

static void cleanname(char *name)
{
	

Mounttab *mtnew(void);
Mounttab *mtcopy(Mounttab *);
void mtref(Mounttab *);
void mtunref(Mounttab *);

void mtadd(Muxmaster *, const char *);
Muxmaster *mtfind(const char *, char *);

