/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2011 The PHP Group                                |
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

#  ifndef PHP_EIO_VERSION
#    define PHP_EIO_VERSION "0.4.0"
#  endif

#  ifdef ZTS
#    include "TSRM.h"
#  endif

PHP_MINIT_FUNCTION(eio);
PHP_MSHUTDOWN_FUNCTION(eio);
PHP_RINIT_FUNCTION(eio);
PHP_RSHUTDOWN_FUNCTION(eio);
PHP_MINFO_FUNCTION(eio);

#endif	/* PHP_EIO_H */
/*
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
