#!/bin/bash -
	#ZEND_DONT_UNLOAD_MODULES=1 \
	#USE_ZEND_ALLOC=1 \
MALLOC_PERTURB_=$(($RANDOM % 255 + 1)) \
	MALLOC_CHECK_=3 \
	php -n -d extension=eio.so  -dextension_dir=./.libs "$@"
