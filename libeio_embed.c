/*
 * Builds libeio as a separate translation unit, so that it only sees the
 * HAVE_* macros detected by libeio/libeio.m4 (via config.h) and not the ones
 * from PHP's php_config.h, which may disagree (e.g. HAVE_FDATASYNC on macOS).
 */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

/* libeio/eio.c uses rename() without including it */
#include <stdio.h>

#include "libeio/eio.c"
