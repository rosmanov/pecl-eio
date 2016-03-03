#!/bin/bash -
phpize
aclocal && libtoolize --force && autoreconf
./configure --enable-eio --enable-eio-debug
make clean
make -j3
