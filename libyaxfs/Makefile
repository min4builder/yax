include ../config.mk

OBJS:=dofunc.o fid.o file.o serve.o

all: headers $(LIBINSTALL)/libyaxfs.a

headers:
	cp -r include/* $(INCLUDEINSTALL)/

$(LIBINSTALL)/libyaxfs.a: libyaxfs.a
	cp libyaxfs.a $(LIBINSTALL)/libyaxfs.a

libyaxfs.a: $(OBJS)
	$(AR) $@ $(OBJS)

clean:
	rm -f *.o *.a

