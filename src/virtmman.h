#ifndef _VIRTMMAN_H
#define _VIRTMMAN_H

#include <stddef.h>
#include <sys/types.h>
#include <yax/mapflags.h>

enum pgfields {
	PGPRESENT = 1 << 0,
	PGWRITEABLE = 1 << 1,
	PGUSER = 1 << 2,
	PGGLOBAL = 1 << 8,
	PGNOSHARE = 1 << 9
};

#define PGADDR 0xfffff000

/* fields inverted because little-endianness */
typedef /*struct {
	uint8_t present : 1;
	uint8_t writable : 1;
	uint8_t user : 1;
	uint8_t writethrough : 1;
	uint8_t cacheoff : 1;
	uint8_t acc : 1;
	uint8_t dirty : 1;
	uint8_t pgsz : 1;
	uint8_t global : 1;
	uint8_t noshare : 1;
	uint8_t avail : 2;
	uint32_t addr : 20;
}*/ uint32_t PgEntry;
typedef PgEntry PgDir[1024];

#define PROT_USER 8

PgDir *vpgnew(void);
void *vpgmap(void *, size_t, enum mapprot, enum mapflags, off_t);
PgDir *vpgcopy(int);
void vpgunmap(void *, size_t);
void vpgclear(void);
void vpgdel(PgDir *);

int verusrptr(const void *, size_t, enum mapprot);
int verusrstr(const char *, enum mapprot);

/* actually declared in boot.s; it needs it to be able to enable paging */
extern PgDir kernel_pd;

#endif /* _VIRTMMAN_H */

