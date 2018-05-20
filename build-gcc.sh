#!/bin/sh
set -e

. ./config.sh

ver=6.1.0

TARGET='elf'

if ! [ -f gcc-$ver.tgz ]; then
	wget -O - http://ftpmirror.gnu.org/gcc/gcc-$ver/gcc-$ver.tar.gz | tee gcc-$ver.tgz | tar -xzf -
elif ! [ -d gcc-$ver ]; then
	tar -xzf gcc-$ver.tgz
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
	../gcc-$ver/configure --target="$YAX_ARCH-$TARGET" --prefix="$YAX_PREFIX" --oldincludedir="$YAX_PREFIX/include" --disable-shared --enable-languages=c --disable-nls --without-headers --disable-lto
	: > .configured
fi

MAKEFLAGS='INFO_DEPS= infodir= MAKEINFO=false'
make -j8 all-gcc $MAKEFLAGS
make -j8 all-target-libgcc $MAKEFLAGS
make -j8 install-gcc $MAKEFLAGS
make -j8 install-target-libgcc $MAKEFLAGS

