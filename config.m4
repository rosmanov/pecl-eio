dnl +----------------------------------------------------------------------+
dnl | PHP Version 5                                                        |
dnl +----------------------------------------------------------------------+
dnl | Copyrght (C) 1997-2012 The PHP Group                                 |
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

dnl {{{ Debug support
if test "$PHP_EIO_DEBUG" != "no"; then
    CFLAGS="$CFLAGS -Wall -g -ggdb -O0"
    AC_DEFINE(EIO_DEBUG,1,[Enable eio debug support])
fi
dnl }}}



dnl {{{ eio support 
if test "$PHP_EIO" != "no"; then

dnl {{{ COMMENTED OUT
dnl     dnl {{{ Include paths
dnl 
dnl     SEARCH_PATH="/usr/local /usr /opt"
dnl     
dnl     dnl {{{ --with-eio
dnl     SEARCH_FOR="include/eio.h"
dnl     if test -r $PHP_EIO/$SEARCH_FOR; then # path given as parameter
dnl         EIO_DIR=$PHP_EIO
dnl     else # search default path list
dnl         AC_MSG_CHECKING([for eio files in default path])
dnl         for i in $SEARCH_PATH ; do
dnl             if test -r $i/$SEARCH_FOR; then
dnl                 EIO_DIR=$i
dnl                 AC_MSG_RESULT(found in $i)
dnl             fi
dnl         done
dnl     fi
dnl     if test -z "$EIO_DIR"; then
dnl         AC_MSG_RESULT([not found])
dnl         AC_MSG_ERROR([Please reinstall libeio])
dnl     fi
dnl     PHP_ADD_INCLUDE($EIO_DIR/include)
dnl     dnl }}}
dnl 
dnl     dnl }}}
dnl 
dnl     dnl {{{ Library checks
dnl     # --with-eio -> check for lib and symbol presence
dnl     LIBNAME=eio
dnl     LIBSYMBOL=eio_init
dnl 
dnl     PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
dnl     [
dnl     PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $EIO_DIR/lib, EIO_SHARED_LIBADD)
dnl     AC_DEFINE(HAVE_EIOLIB,1,[ ])
dnl     ],[
dnl     AC_MSG_ERROR([wrong eio lib version or lib not found])
dnl     ],[
dnl     -L$EIO_DIR/lib
dnl     ])
dnl 
dnl     PHP_SUBST(EIO_SHARED_LIBADD)
dnl     dnl }}}
dnl
dnl     dnl }}}

    dnl AC_CHECK_HEADERS(sys/eventfd.h linux/falloc.h)
    dnl AC_CHECK_FUNCS(eventfd fallocate)
    
    PHP_ADD_INCLUDE(.)
    AC_CONFIG_SRCDIR([libeio/eio.h])
    dnl AC_CONFIG_HEADERS([config.h])
    m4_include([libeio/libeio.m4])

    dnl Build extension 
    eio_src="php_eio.c eio_fe.c"
    PHP_NEW_EXTENSION(eio, $eio_src, $ext_shared,,$CFLAGS)
    PHP_ADD_EXTENSION_DEP(eio, sockets, true)
fi
dnl }}}

dnl vim: ft=m4.sh et fdm=marker cms=dnl\ %s
