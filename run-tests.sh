#!/bin/sh -
set -x

dir="$(cd "$(dirname "$0")" || exit 1; pwd)"
php_executable="${php_executable:-php}"

TEST_PHP_EXECUTABLE="${dir}/test-php.sh" "${php_executable}" -n "${dir}/run-tests.php" "$@"
