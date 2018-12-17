#CC=cparser -target $(YAX_ARCH)-yax
CC=$(YAX_ARCH)-yax-gcc
LD=$(CC)
AR=$(YAX_ARCH)-yax-ar crs
ASM=nasm -felf32 -g
CFLAGS=-g -Og -Wall -Wextra -Werror -Wno-error=switch -Wno-error=enum-compare -Wno-error=parentheses
LDFLAGS=
LIBS=

LIBINSTALL=../sysroot/lib
INCLUDEINSTALL=../sysroot/include

.s.o: $*.s
	$(ASM) -o $@ $<

.c: $*.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
.o: $*.o
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS)

