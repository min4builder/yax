#ifndef _BOOT_H
#define _BOOT_H
#include <stdint.h>

typedef struct {
	uint32_t size;
	uint64_t base;
	uint64_t len;
	uint32_t type;
} MemoryMap;

typedef struct {
	uint8_t *start;
	uint8_t *end;
	char *string;
	uint32_t reserved;
} Module;

typedef struct {
	uint32_t flags;
	uint32_t lmem, umem;
	uint32_t bootdev;
	char *cmdline;
	uint32_t modn;
	Module *mods;
	uint32_t syms[4];
	uint32_t mmaplen;
	MemoryMap *mmap;
} MultibootInfo;

extern void *(*switchsyscallstack)(void *);

void halt(void);

void clearkidmap(void);

void kernel_main(MultibootInfo *);

/* ugly hack; see boot.s for reason */
extern uint8_t VIRT;
#define VIRT(x) (&VIRT + (uint32_t)(x))
#define PHYS(x) ((uint8_t *)(x) - &VIRT)

extern uint8_t kernel_stack_bottom;
extern uint8_t kernel_stack_low;
#define kernel_stack_bottom ((void *) &kernel_stack_bottom)
#define kernel_stack_low ((void *) &kernel_stack_low)

/* ugly linker hacks; see linker.ld */
extern uint8_t RX_SECTIONS, RX_SECTIONS_LEN,
            RO_SECTIONS, RO_SECTIONS_LEN,
            RW_SECTIONS, RW_SECTIONS_LEN;
#define RX_SECTIONS (&RX_SECTIONS)
#define RX_SECTIONS_LEN ((off_t) &RX_SECTIONS_LEN)
#define RO_SECTIONS (&RO_SECTIONS)
#define RO_SECTIONS_LEN ((off_t) &RO_SECTIONS_LEN)
#define RW_SECTIONS (&RW_SECTIONS)
#define RW_SECTIONS_LEN ((off_t) &RW_SECTIONS_LEN)

extern uint8_t KERNEL_LOAD, KERNEL_LOAD_END;
#define KERNEL_LOAD ((uintptr_t) &KERNEL_LOAD)
#define KERNEL_LOAD_END ((uintptr_t) &KERNEL_LOAD_END)

#endif /* _BOOT_H */

