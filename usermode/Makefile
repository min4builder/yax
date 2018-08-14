#CC=cparser -target i686-yax
CC=i686-elf-gcc
#LD=i686-yax-ld
LD=$(CC)
AR=i686-yax-ar crs
ASM=nasm -felf32 -g
CFLAGS=-g -O0 -ffreestanding -I../include #-Wall -Wextra -Werror
LDFLAGS=-L. -nostdlib crt0.o crti.o -T linker.ld
LIBS=-lc crtn.o -lgcc

OBJS=_libc_main.o abort.o environ.o errno.o exit.o liballoc.o memcpy.o memcmp.o memset.o port.o serve.o strcat.o strcmp.o strcpy.o strlen.o strlcpy.o strncat.o syscall.o

initrd: testcode.o tarfs.o initramdisk.o libc.a crt0.o crti.o crtn.o
	$(LD) $(LDFLAGS) -o $@ testcode.o tarfs.o initramdisk.o $(LIBS)

initramdisk.o: sf

#sf: testexec.o libc.a crt0.o crti.o crtn.o
#	$(LD) $(LDFLAGS) -o $@ testexec.o $(LIBS)

libc.a: $(OBJS)
	$(AR) -o $@ $(OBJS)

clean:
	rm -f *.o *.a initrd

.s.o: $*.s
	$(ASM) $<

.c.o: $*.c
	$(CC) $(CFLAGS) -o $@ -c $<
	
