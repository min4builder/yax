#CC=cparser -target i686-elf
CC=i686-elf-gcc -fstack-protector-all
#LD=i686-elf-ld
LD=$(CC) -ffreestanding
ASM=nasm -felf32 -g
CFLAGS=-g -O3 -Wall -Wextra -pedantic
LDFLAGS=
CFLAGS+=-I../include -std=c99 -ffreestanding
LDFLAGS+=-T linker.ld -nostdlib
LIBS=-lgcc

# boot.o must be first
OBJS=boot.o \
     conn.o exec.o fds.o int.o libk.o lock.o main.o malloc.o multitask.o multitasks.o pgdir.o \
     physmman.o pic.o pipe.o pit.o port.o printk.o ssp.o stat.o syscall.o sysentry.o virtmman.o

all: yax.bin ../usermode/initrd

clean:
	-$(RM) -f *.o depend.mk yax.bin yax.img yax.sym

yax.img: yax.bin
	mkfs.msdos -C $@ 1440
	mcopy -i $@ yax.bin syslinux.cfg ../usermode/initrd ::
	syslinux $@

yax.sym: yax.bin
	i686-elf-nm -B yax.bin > $@

test: yax.bin ../usermode/initrd
	qemu-system-i386 -serial stdio -kernel yax.bin -initrd ../usermode/initrd 2>/dev/null

debug: yax.bin ../usermode/initrd
	qemu-system-i386 -S -s -serial stdio -kernel yax.bin -initrd ../usermode/initrd 2>/dev/null

bochs: yax.img yax.sym
	bochs -q

yax.bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o: $*.c
	$(CC) $(CFLAGS) -o $@ -c $<

.s.o: $*.s
	$(ASM) -o $@ $<

include depgen.mk

