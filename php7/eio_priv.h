/*
   +----------------------------------------------------------------------+
   | PHP Version 7                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2017 The PHP Group                                |
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

#ifndef EIO_PRIV_H
#  define EIO_PRIV_H

extern const zend_function_entry eio_functions[];

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif


#define PHP_EIO_GRP_DESCRIPTOR_NAME "EIO Group Descriptor"
#define PHP_EIO_REQ_DESCRIPTOR_NAME "EIO Request Descriptor"

/* {{{ Macros */

#ifdef EIO_DEBUG
# define PHP_EIO_RET_IF_FAILED(req, eio_func) \
	if (!req || (req->result != 0 && req->errorno)) { \
		php_error_docref(NULL, \
				E_WARNING, #eio_func " failed: %s", strerror(req->errorno)); \
		RETURN_FALSE; \
	}
#else
# define PHP_EIO_RET_IF_FAILED(req, eio_func) \
	if (!req || req->result != 0) RETURN_FALSE;
#endif

#define PHP_EIO_RET_REQ_RESOURCE(req, eio_func) do { \
	PHP_EIO_RET_IF_FAILED(req, eio_func); \
	RETURN_RES(zend_register_resource(req, le_eio_req)); \
} while (0)

#define PHP_EIO_IS_INIT() \
{ \
	if (php_eio_pid <= 0 || php_eio_pipe.len == 0) { \
		php_eio_init(); \
	} \
}

#define PHP_EIO_SETFD_CLOEXEC(fd) \
{ \
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) { \
		php_error_docref(NULL, \
				E_WARNING, "Failed to set FD_CLOEXEC on descriptor"); \
	} \
}


#define PHP_EIO_INIT \
	zend_long           pri    = EIO_PRI_DEFAULT; \
	zval               *zcb    = NULL;            \
	zval               *data   = NULL;            \
	php_eio_cb_t       *eio_cb;                   \
	eio_req            *req;                      \
	PHP_EIO_IS_INIT();

#ifdef EIO_DEBUG
# define EIO_CHECK_PATH_LEN(path, path_len) \
	if (strlen(path) != path_len) { \
		php_error_docref(NULL, E_WARNING, \
				"failed calculating path length"); \
		RETURN_FALSE; \
	}
#else
# define EIO_CHECK_PATH_LEN(path, path_len) \
	if (strlen(path) != path_len) { \
		RETURN_FALSE; \
	}
#endif

#define EIO_REGISTER_LONG_EIO_CONSTANT(name) \
	REGISTER_LONG_CONSTANT(#name, name, CONST_CS | CONST_PERSISTENT);

#define EIO_REGISTER_LONG_CONSTANT(name, value) \
	REGISTER_LONG_CONSTANT(#name, value, CONST_CS | CONST_PERSISTENT);

#define EIO_CB_CUSTOM_IS_LOCKED(eio_cb) ((eio_cb) ? (eio_cb)->locked : 0)

/* }}} */

/* {{{ Types */

typedef struct _php_eio_func_info {
	zend_function    *func_ptr;
	zend_class_entry *ce;
	zval              obj;
	zval              closure;
} php_eio_func_info;

typedef struct {
	php_eio_func_info func;
	zval              arg;    /* callback argument */
#ifdef ZTS
	void*** ls;
#endif
} php_eio_cb_t;

typedef struct {
	zval              arg;         /* callback argument */
	zend_bool         locked;
	php_eio_func_info func_exec;
	php_eio_func_info func;
#ifdef ZTS
	void*** ls;
#endif
} php_eio_cb_custom_t;

typedef struct {
	int fd[2];
	int len; /* 1 for pipe, 8 for eventfd */
} php_eio_pipe_t;

/* }}} */

#endif	/* EIO_PRIV_H */
/*
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
