#!/bin/bash -
# Set PECL package version

set -e

if [[ $# < 1 ]]; then
	echo >&2 'Version string expected'
	exit 1
fi

php_eio_version="$1"

perl -pi -e 's/(# *define *PHP_EIO_VERSION\ +).*/$1"'$php_eio_version'"/g' php{5,7,8}/php_eio.h

echo "Done"
echo "(!) Don't forget to update package.xml"
