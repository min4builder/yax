include ../config.mk

LIBS+=-lyaxfs

all: initrd

initrd: main.o tarfs.o initramdisk.o
	$(LD) $(LDFLAGS) -o $@ main.o tarfs.o initramdisk.o $(LIBS)

initramdisk.o: sf

sf: init ../drivers/cons ../drivers/disk
	mkdir it; mkdir it/dev; mkdir it/bin
	cp init it/bin/init
	cp ../drivers/cons it/bin/cons
	cp ../drivers/disk it/bin/disk
	cd it; tar cvf $(PWD)/$@ *
	rm -rf it

clean:
	rm -f *.o *.a initrd sf init

