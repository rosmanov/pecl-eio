#!/bin/sh -
set -x

dir=$(cd $(dirname "$0"); pwd)
: ${php_executable:=php}

TEST_PHP_EXECUTABLE="${dir}/test-php.sh" "${php_executable}" "${dir}/run-tests.php"
