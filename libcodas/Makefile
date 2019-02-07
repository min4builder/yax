include ../config.mk

OBJS:=ref.o vector.o

all: headers $(LIBINSTALL)/libcodas.a

headers:
	cp -r include/* $(INCLUDEINSTALL)/

$(LIBINSTALL)/libcodas.a: libcodas.a
	cp libcodas.a $@

libcodas.a: $(OBJS)
	$(AR) -o $@ $(OBJS)

clean:
	rm -f *.o *.a

