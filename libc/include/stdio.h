#include <_yalc.h>

#ifndef _STDIO_H_ANSI
#define _STDIO_H_ANSI

/* exception to the no-includes-on-headers rule */
#include <stdarg.h>

#ifndef NULL
#define NULL _YalcNULL
#endif

#define BUFSIZ 1024

#define EOF -1

#define _IONBF -1
#define _IOFBF 0
#define _IOLBF 1

_YALCDEFTYPE(fpos_t);
_YALCDEFTYPE(off_t);
_YALCDEFTYPE(size_t);

typedef struct _YALC_FILE FILE;

extern FILE *const stdin, *const stdout, *const stderr;

int fclose(FILE *);
int ferror(FILE *);
int fflush(FILE *);
int fgetc(FILE *);
FILE *fopen(char const *, char const *);
int fprintf(FILE *__restrict, char const *__restrict, ...);
int fputc(int, FILE *);
int fputs(char const *__restrict, FILE *__restrict);
size_t fread(void *__restrict, size_t, size_t, FILE *__restrict);
size_t fwrite(void const *__restrict, size_t, size_t, FILE *__restrict);
int printf(char const *, ...);
int puts(char const *);
int vfprintf(FILE *__restrict, char const *__restrict, va_list);

#define stdin (stdin)
#define stdout (stdout)
#define stderr (stderr)
#define putc fputc

#endif /* _STDIO_H_ANSI */


#if !defined(_STDIO_H_POSIX) && defined(_YALC_NEED_POSIX)
#define _STDIO_H_POSIX

_YALCDEFTYPE(ssize_t);

FILE *fdopen(int, char const *);

#endif /* _STDIO_H_POSIX */

