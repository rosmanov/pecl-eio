#!/bin/sh -
# Can be used to run tests when `make test` fails because of the extension loading order issues.
# E.g.: TEST_PHP_EXECUTABLE="$(pwd)/run-tests.sh" php run-tests.php

dir="$(cd "$(dirname "$0")" && pwd)"
local_libs_dir="${dir}/.libs"


if [ ! -e "${local_libs_dir}/sockets.so" ]; then
    sockets_so_path="$(php-config --extension-dir)/sockets.so"
    cp "$sockets_so_path" "${local_libs_dir}"
fi

if [ ! -e "${local_libs_dir}/pcntl.so" ]; then
    pcntl_so_path="$(php-config --extension-dir)/pcntl.so"
    if [ -f "$pcntl_so_path" ]; then
        cp "$pcntl_so_path" "${local_libs_dir}"
    fi
fi

if [ "${SANITIZER:-none}" = "asan" ]; then
    libasan="$(gcc -print-file-name=libasan.so)"

    exec env \
        LD_PRELOAD="$libasan" \
        ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0:abort_on_error=1}" \
        php -n \
            -d extension=sockets.so \
            -d extension=pcntl.so \
            -d extension=eio.so \
            -d extension_dir="${local_libs_dir}" \
            "$@"
else
    exec php -n \
        -d extension=sockets.so \
        -d extension=pcntl.so \
        -d extension=eio.so \
        -d extension_dir="${local_libs_dir}" \
        "$@"
fi
