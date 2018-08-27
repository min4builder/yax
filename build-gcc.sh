#!/bin/sh
set -e

. ./config.sh

ver=8.2.0

TARGET='yax'

if ! [ -f gcc-$ver.tgz ]; then
	wget -O - http://ftpmirror.gnu.org/gcc/gcc-$ver/gcc-$ver.tar.gz | tee gcc-$ver.tgz | tar -xzf -
elif ! [ -d gcc-$ver ]; then
	tar -xzf gcc-$ver.tgz
	cd gcc-$ver
	patch -p1 < ../gcc-$ver.diff
	cd ..
fi

cd gcc-$ver

if ! test -f .downloaded; then
	contrib/download_prerequisites
	: > .downloaded
fi

cd ..

mkdir -p build-$YAX_ARCH-gcc-$ver
cd build-$YAX_ARCH-gcc-$ver

if ! test -f .configured; then
	../gcc-$ver/configure --target="$YAX_ARCH-$TARGET" --prefix="$YAX_PREFIX" --oldincludedir="/include" --disable-shared --enable-languages=c --disable-nls --without-headers --disable-lto --with-sysroot="$YAX_PREFIX"
	: > .configured
fi

MAKEFLAGS='INFO_DEPS= infodir= MAKEINFO=false'
make -j8 all-gcc $MAKEFLAGS
make -j8 all-target-libgcc $MAKEFLAGS
make -j8 install-gcc DESTDIR="$YAX_PREFIX" $MAKEFLAGS
make -j8 install-target-libgcc DESTDIR="$YAX_PREFIX" $MAKEFLAGS

