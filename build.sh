#!/bin/sh

old_CFLAGS="$CFLAGS"
# Note, -Wformat-overflow values > 0 only trigger errors when optimization mode (-O) is turned on (> 0).
export CFLAGS="$CFLAGS -g -O2 -Wformat-overflow=2"

phpize
./configure --with-eio "$@"
make

export CFLAGS="$old_CFLAGS"
