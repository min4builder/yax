#!/bin/sh
set -e

. ./config.sh

ver=2.28

TARGET='yax'

if ! [ -f binutils-$ver.tgz ]; then
	wget -O - http://ftp.gnu.org/gnu/binutils/binutils-$ver.tar.gz | tee binutils-$ver.tgz | tar -xzf -
elif ! [ -d binutils-$ver ]; then
	tar -xzf binutils-$ver.tgz
	cd binutils-$ver
	patch -p1 < ../binutils-$ver.diff
	cd ..
fi

mkdir build-$ARCH-binutils-$ver
cd build-$ARCH-binutils-$ver

../binutils-$ver/configure --target="$YAX_ARCH-$TARGET" --prefix="$YAX_PREFIX" --with-sysroot --disable-shared --disable-multilib --disable-nls
make -j8
make install

