#!/bin/sh
set -e

. ./config.sh

ver=2.28

TARGET='yax'

if ! [ -f binutils-$ver.tgz ]; then
	wget -O - http://ftp.gnu.org/gnu/binutils/binutils-$ver.tar.gz | tee binutils-$ver.tgz | tar -xzf -
	cd binutils-$ver
	patch -p1 < ../binutils-$ver.diff
	cd ..
elif ! [ -d binutils-$ver ]; then
	tar -xzf binutils-$ver.tgz
	cd binutils-$ver
	patch -p1 < ../binutils-$ver.diff
	cd ..
fi

mkdir -p build-$YAX_ARCH-binutils-$ver
cd build-$YAX_ARCH-binutils-$ver

../binutils-$ver/configure --target="$YAX_ARCH-$TARGET" --prefix="/" --with-sysroot="$YAX_PREFIX" --disable-shared --disable-multilib --disable-nls
make -j8
make install DESTDIR="$YAX_PREFIX"

