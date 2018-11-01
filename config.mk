#CC=cparser -target i686-yax
CC=i686-yax-gcc
LD=$(CC)
AR=i686-yax-ar crs
ASM=nasm -felf32 -g
CFLAGS=-g -O3 -Wall -Wextra -Werror -Wno-error=switch -Wno-error=enum-compare
LDFLAGS=
LIBS=

LIBINSTALL=../sysroot/lib
INCLUDEINSTALL=../sysroot/include

.s.o: $*.s
	$(ASM) -o $@ $<

