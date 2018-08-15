#!/bin/sh
set -e

. ./config.sh

if ! [ -d cparser ]; then
	git clone --recursive https://pp.ipd.kit.edu/git/cparser.git
	echo "PREFIX='$(echo "$YAX_PREFIX" | sed "s/'/'\\\\''/g")'
SYSTEM_INCLUDE_DIR=\$(PREFIX)/include
LOCAL_INCLUDE_DIR=\$(SYSTEM_INCLUDE_DIR)
SYSTEM_LIB_DIR=\$(PREFIX)/lib
LOCAL_LIB_DIR=\$(SYSTEM_LIB_DIR)
HOST_TRIPLE=$YAX_ARCH-elf" > cparser/config.mak
	cp cparser-stdint.h cparser/include/stdint.h
fi

cd cparser
make
make install

