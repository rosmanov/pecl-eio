/*
   +----------------------------------------------------------------------+
   | PHP Version 7                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2021 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Ruslan Osmanov <osmanov@php.net>                             |
   +----------------------------------------------------------------------+
*/

#ifndef PHP_EIO_H
#  define PHP_EIO_H

extern zend_module_entry eio_module_entry;
#  define phpext_eio_ptr &eio_module_entry

#ifndef PHP_EIO_VERSION
# define PHP_EIO_VERSION "3.0.0RC1"
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

/* zend_fcall_info.symbol_table removed from PHP 7.1.x */
#if PHP_VERSION_ID < 70100
# define HAVE_PHP_ZEND_FCALL_INFO_SYMBOL_TABLE 1
# define HAVE_PHP_ZEND_FCALL_INFO_FUNCTION_TABLE 1
#endif

PHP_MINIT_FUNCTION(eio);
PHP_MSHUTDOWN_FUNCTION(eio);
PHP_RINIT_FUNCTION(eio);
PHP_RSHUTDOWN_FUNCTION(eio);
PHP_MINFO_FUNCTION(eio);

#if defined(COMPILE_DL_EIO)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

#endif /* PHP_EIO_H */
/*
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
