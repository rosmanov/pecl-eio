#!/bin/bash -
phpize --clean
phpize
#aclocal && libtoolize --force && autoreconf

old_CFLAGS="$CFLAGS"
# Note, -Wformat-overflow values > 0 only trigger errors when optimization mode (-O) is turned on (> 0).
export CFLAGS="$CFLAGS -g -O2 -Wformat-overflow=2"

./configure --with-eio "$@"
make clean
make -j3

export CFLAGS="$old_CFLAGS"
