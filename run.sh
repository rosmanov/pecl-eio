#!/bin/bash -
set -x

dir="$(cd "$(dirname "$0")"; pwd)"

sockets_so_path="$(php-config --extension-dir)/sockets.so"
sockets_option=
if [ -e "$sockets_so_path" ]; then
  cp "$sockets_so_path" "${dir}/.libs/"
  sockets_option='-dextension=sockets.so'
fi

MALLOC_PERTURB_=$(($RANDOM % 255 + 1)) \
	MALLOC_CHECK_=3 \
	USE_ZEND_ALLOC=0 \
	ZEND_DONT_UNLOAD_MODULES=1 \
    php -n  $sockets_option -d extension=eio.so  -dextension_dir="${dir}/.libs" "$@"
