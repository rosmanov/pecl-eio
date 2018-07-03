PHP_ARG_WITH(eio, for eio support,
[  --with-eio               Include eio support])

PHP_ARG_ENABLE(eio-debug, for eio debug support,
[  --enable-eio-debug       Enable eio debug support], no, no)

AC_CHECK_HEADERS(sys/eventfd.h linux/falloc.h)
AC_CHECK_FUNCS(eventfd)

 Debug support
if test "$PHP_EIO_DEBUG" != "no"; then
    EXTRA_CFLAGS="$EXTRA_CFLAGS -Wall -g -ggdb -O0 -fno-omit-frame-pointer"
    AC_DEFINE(EIO_DEBUG,1,[Enable eio debug support])
fi



 eio support
if test "$PHP_EIO" != "no"; then
    if test "$ext_shared" != "yes" && test "$ext_shared" != "shared"; then
      PHP_EIO_CONFIG_H='\"main/php_config.h\"'
      AC_DEFINE(EIO_CONFIG_H, "main/php_config.h", [Overide config.h included in libeio/eio.c])
      EXTRA_CFLAGS="$EXTRA_CFLAGS -DEIO_CONFIG_H="$PHP_EIO_CONFIG_H
      define('PHP_EIO_STATIC', 1)
    fi

    AC_MSG_CHECKING(PHP version)
    if test -d $abs_srcdir/php7; then
        dnl # only for PECL, not for PHP
        export OLD_CPPFLAGS="$CPPFLAGS"
        export CPPFLAGS="$CPPFLAGS $INCLUDES"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <php_version.h>]], [[
            #if PHP_MAJOR_VERSION > 5
            # error PHP > 5
            #endif
        ]])],[
            subdir=php5
            AC_MSG_RESULT([PHP 5.x])
        ],[
            subdir=php7
            AC_MSG_RESULT([PHP 7.x])
        ])
        export CPPFLAGS="$OLD_CPPFLAGS"
        PHP_EIO_SOURCES="$subdir/php_eio.c $subdir/eio_fe.c"
    else
        AC_MSG_ERROR([unknown source])
        PHP_EIO_SOURCES="php_eio.c eio_fe.c"
    fi

    if test -n "$subdir"; then
        PHP_ADD_BUILD_DIR($abs_builddir/$subdir, 1)
        PHP_ADD_INCLUDE([$ext_srcdir/$subdir])
    fi

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


dnl vim: ft=m4.sh et fdm=marker cms=dnl\ %s
