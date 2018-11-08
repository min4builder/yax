#CC=cparser -target $(YAX_ARCH)-yax
CC=$(YAX_ARCH)-yax-gcc
LD=$(CC)
AR=$(YAX_ARCH)-yax-ar crs
ASM=nasm -felf32 -g
CFLAGS=-g -O3 -Wall -Wextra -Werror -Wno-error=switch -Wno-error=enum-compare
LDFLAGS=
LIBS=

LIBINSTALL=../sysroot/lib
INCLUDEINSTALL=../sysroot/include

.s.o: $*.s
	$(ASM) -o $@ $<

