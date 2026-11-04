#!/bin/bash -
phpize --clean
phpize
#aclocal && libtoolize --force && autoreconf

old_CFLAGS="$CFLAGS"
# Note, -Wformat-overflow values > 0 only trigger errors when optimization mode (-O) is turned on (> 0).
# The option is not supported by every compiler (e.g. GCC < 7, clang), so probe it first.
extra_cflags="-g -O2"
if echo 'int main(void) { return 0; }' | ${CC:-cc} -Werror -Wformat-overflow=2 -x c -fsyntax-only - >/dev/null 2>&1; then
    extra_cflags="$extra_cflags -Wformat-overflow=2"
fi
export CFLAGS="$CFLAGS $extra_cflags"

./configure --with-eio "$@"
make clean

if command -p nproc >/dev/null 2>&1; then
    num_parallel="$(nproc)"
else
    num_parallel="$(getconf _NPROCESSORS_ONLN)"
fi
if [ -z "$num_parallel" ]; then
    num_parallel=1
fi
make -j"${num_parallel}"
export CFLAGS="$old_CFLAGS"
