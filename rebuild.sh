#!/bin/bash -
phpize --clean
phpize
#aclocal && libtoolize --force && autoreconf
./configure --with-eio --enable-eio-debug
make clean
make -j3
