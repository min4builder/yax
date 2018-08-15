include ../config.mk

all: initrd

initrd: main.o tarfs.o initramdisk.o
	$(LD) $(LDFLAGS) -o $@ main.o tarfs.o initramdisk.o $(LIBS)

initramdisk.o: sf

#sf: testexec.o libc.a crt0.o crti.o crtn.o
#	$(LD) $(LDFLAGS) -o $@ testexec.o $(LIBS)

clean:
	rm -f *.o *.a initrd sf

