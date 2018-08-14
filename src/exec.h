#ifndef _EXEC_H
#define _EXEC_H

#include <stdint.h>
#include <sys/types.h>

uint32_t execmod(void *, size_t, void **, char *, char *);
uint32_t exec(Conn *, void **, char *, char *);

#endif /* _EXEC_H */

