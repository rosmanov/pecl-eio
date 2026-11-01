#!/bin/sh -
# Can be used to run tests when `make test` fails because of the extension loading order issues.
# E.g.: TEST_PHP_EXECUTABLE="$(pwd)/run-tests.sh" php run-tests.php

dir="$(cd "$(dirname "$0")" && pwd)"
local_libs_dir="${dir}/.libs"

# Optional extensions used by the tests. They can be built into PHP (no .so), or not installed at all.
for ext in sockets pcntl; do
    ext_so_path="$(php-config --extension-dir)/${ext}.so"
    if [ ! -e "${local_libs_dir}/${ext}.so" ] && [ -f "$ext_so_path" ]; then
        cp "$ext_so_path" "${local_libs_dir}"
    fi
done

# The order matters: eio depends on sockets. The loop below prepends, so sockets ends up first.
set -- -d extension=eio.so -d extension_dir="${local_libs_dir}" "$@"
for ext in pcntl sockets; do
    if [ -e "${local_libs_dir}/${ext}.so" ]; then
        set -- -d "extension=${ext}.so" "$@"
    fi
done

if [ "${SANITIZER:-none}" = "asan" ]; then
    libasan="$(gcc -print-file-name=libasan.so)"

    exec env \
        LD_PRELOAD="$libasan" \
        ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0:abort_on_error=1}" \
        php -n "$@"
else
    exec php -n "$@"
fi
