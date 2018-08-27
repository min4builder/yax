#ifndef _STDIO_H
#define _STDIO_H

typedef int FILE;

extern FILE *stderr;

int fflush(FILE *);
int fprintf(FILE *, const char *, ...);

#endif /* _STDIO_H */

