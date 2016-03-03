#!/bin/bash -
MALLOC_PERTURB_=$(($RANDOM % 255 + 1)) \
	MALLOC_CHECK_=3 \
	USE_ZEND_ALLOC=0 \
	ZEND_DONT_UNLOAD_MODULES=1 \
	php -n -d extension=eio.so  -dextension_dir=./.libs "$@"
