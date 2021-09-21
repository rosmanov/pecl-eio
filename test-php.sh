#!/bin/sh -
# Can be used to run tests when `make test` fails because of the extension loading order issues.
# E.g.: TEST_PHP_EXECUTABLE="$(pwd)/run-tests.sh" php run-tests.php

dir=$(cd $(dirname "$0"); pwd)
local_libs_dir="${dir}/.libs"

php -n -dextension=eio.so -dextension_dir="${local_libs_dir}" "$@"
