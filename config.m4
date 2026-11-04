dnl +----------------------------------------------------------------------+
dnl | PHP Version 8                                                        |
dnl +----------------------------------------------------------------------+
dnl | Copyright (C) 1997-2026 The PHP Group                                |
dnl +----------------------------------------------------------------------+
dnl | This source file is subject to version 3.01 of the PHP license,      |
dnl | that is bundled with this package in the file LICENSE, and is        |
dnl | available through the world-wide-web at the following url:           |
dnl | http://www.php.net/license/3_01.txt                                  |
dnl | If you did not receive a copy of the PHP license and are unable to   |
dnl | obtain it through the world-wide-web, please send a note to          |
dnl | license@php.net so we can mail you a copy immediately.               |
dnl +----------------------------------------------------------------------+
dnl | Author: Ruslan Osmanov <osmanov@php.net>                             |
dnl +----------------------------------------------------------------------+

PHP_ARG_WITH(eio, for eio support,
[  --with-eio               Include eio support])

PHP_ARG_ENABLE(eio-debug, for eio debug support,
[  --enable-eio-debug       Enable eio debug support], no, no)

AC_CHECK_HEADERS(sys/eventfd.h linux/falloc.h)
AC_CHECK_FUNCS(eventfd)

dnl Debug support
if test "$PHP_EIO_DEBUG" != "no"; then
    EXTRA_CFLAGS="$EXTRA_CFLAGS -Wall -g -ggdb -O0 -fno-omit-frame-pointer -fno-stack-protector"
    AC_DEFINE(EIO_DEBUG,1,[Enable eio debug support])
fi

dnl eio support
if test "$PHP_EIO" != "no"; then
    if test "$ext_shared" != "yes" && test "$ext_shared" != "shared"; then
      PHP_EIO_CONFIG_H='\"main/php_config.h\"'
      AC_DEFINE(EIO_CONFIG_H, "main/php_config.h", [Overide config.h included in libeio/eio.c])
      EXTRA_CFLAGS="$EXTRA_CFLAGS -DEIO_CONFIG_H="$PHP_EIO_CONFIG_H
      define('PHP_EIO_STATIC', 1)
    fi

    dnl Enable optional sockets integration when the development headers are available.
    AC_MSG_CHECKING([for PHP sockets headers])
    AS_IF([test -f "$phpincludedir/ext/sockets/php_sockets.h"], [
        AC_MSG_RESULT([yes])
        AC_DEFINE([HAVE_SOCKETS], [1],
            [Define to 1 if PHP sockets headers are available])
    ], [
        AC_MSG_RESULT([no])
    ])

    dnl {{{ Check for PHP version
    AC_MSG_CHECKING(PHP >= 8.2)
    export OLD_CPPFLAGS="$CPPFLAGS"
    export CPPFLAGS="$CPPFLAGS $INCLUDES"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <php_version.h>]], [[
        #if PHP_VERSION_ID < 80200
        # error PHP < 8.2
        #endif
    ]])],
        [AC_MSG_RESULT([yes])],
        [AC_MSG_ERROR([PHP 8.2 or newer is required])])
    export CPPFLAGS="$OLD_CPPFLAGS"
    dnl }}}

    PHP_EIO_SUBDIR=php8
    PHP_EIO_SOURCES="$PHP_EIO_SUBDIR/php_eio.c libeio_embed.c"

    PHP_ADD_BUILD_DIR($abs_builddir/$PHP_EIO_SUBDIR, 1)
    PHP_ADD_INCLUDE([$ext_srcdir/$PHP_EIO_SUBDIR])

    PHP_ADD_INCLUDE($ext_builddir)
    AC_CONFIG_SRCDIR(ifdef('PHP_EIO_STATIC',PHP_EXT_BUILDDIR(eio)[/],)[libeio/eio.h])
    AC_DEFINE(EIO_STACKSIZE, [262144], [ Stack size limit for libeio ])
    m4_include(ifdef('PHP_EIO_STATIC',PHP_EXT_BUILDDIR(eio)[/],)[libeio/libeio.m4])

    PHP_NEW_EXTENSION(eio, $PHP_EIO_SOURCES, $ext_shared,,-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
    PHP_ADD_EXTENSION_DEP(eio, sockets, true)

    dnl Bug #4(linking issue on aarch64)
    EXTRA_LDFLAGS="$EXTRA_LDFLAGS -pthread"

    PHP_SUBST(EXTRA_LDFLAGS)
    PHP_SUBST(EXTRA_CFLAGS)
	PHP_ADD_MAKEFILE_FRAGMENT
fi

dnl vim: fdm=marker cms=dnl\ %s
dnl vim: et ts=2 sts=2 sw=2
