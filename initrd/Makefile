include ../config.mk

all: initrd

initrd: main.o tarfs.o initramdisk.o
	$(LD) $(LDFLAGS) -o $@ main.o tarfs.o initramdisk.o $(LIBS)

initramdisk.o: sf

sf: init ../drivers/cons
	mkdir it; mkdir it/dev; mkdir it/bin
	cp init it/bin/init
	cp ../drivers/cons it/bin/cons
	cd it; tar cvf $(PWD)/$@ *
	rm -rf it

clean:
	rm -f *.o *.a initrd sf

