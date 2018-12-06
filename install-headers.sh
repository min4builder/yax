#!/bin/sh

. ./config.sh

mkdir -p "$YAX_PREFIX/include/yax"
cp -r src/include/* "$YAX_PREFIX/include/yax"
cp -r libc/include/* "$YAX_PREFIX/include"
cp -r libcodas/include/* "$YAX_PREFIX/include"
cp -r libyaxfs/include/* "$YAX_PREFIX/include"

