/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
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

#  define PHP_EIO_GRP_DESCRIPTOR_NAME "EIO Group Descriptor"
#  define PHP_EIO_REQ_DESCRIPTOR_NAME "EIO Request Descriptor"

/* {{{ Macros */

#  ifdef ZTS
#    define TSRMLS_FETCH_FROM_CTX(ctx) void ***tsrm_ls = (void ***) ctx
#    define TSRMLS_SET_CTX(ctx) ctx = (void ***) tsrm_ls
#  else
#    define TSRMLS_FETCH_FROM_CTX(ctx)
#    define TSRMLS_SET_CTX(ctx)
#  endif

#  ifdef EIO_DEBUG
#    define PHP_EIO_RET_IF_FAILED(req, eio_func) \
    	if (!req || (req->result != 0 && req->errorno)) { \
			php_error_docref(NULL TSRMLS_CC, \
				E_WARNING, #eio_func " failed: %s", strerror(req->errorno)); \
			RETURN_FALSE; \
    	}
#  else
#    define PHP_EIO_RET_IF_FAILED(req, eio_func) \
    	if (!req || req->result != 0) RETURN_FALSE;
#  endif

#  define PHP_EIO_RET_REQ_RESOURCE(req, eio_func) \
    PHP_EIO_RET_IF_FAILED(req, eio_func); \
	ZEND_REGISTER_RESOURCE(return_value, req, le_eio_req);

#  define PHP_EIO_IS_INIT() \
{ \
	if (php_eio_pid <= 0 || php_eio_pipe.len == 0) { \
		php_eio_init(TSRMLS_C); \
	} \
}

#define PHP_EIO_SETFD_CLOEXEC(fd) \
{ \
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) { \
		php_error_docref(NULL TSRMLS_CC, \
				E_WARNING, "Failed to set FD_CLOEXEC on descriptor"); \
	} \
}


#  define PHP_EIO_INIT \
	long pri                  = EIO_PRI_DEFAULT; \
	zval *data                = NULL; \
	zend_fcall_info fci       = empty_fcall_info; \
	zend_fcall_info_cache fcc = empty_fcall_info_cache; \
	php_eio_cb_t *eio_cb; \
	eio_req *req; \
	PHP_EIO_IS_INIT();

#  ifdef EIO_DEBUG
#    define EIO_CHECK_PATH_LEN(path, path_len) \
    if (strlen(path) != path_len) { \
	php_error_docref(NULL TSRMLS_CC, E_WARNING, \
		"failed calculating path length"); \
	RETURN_FALSE; \
    }
#  else
#    define EIO_CHECK_PATH_LEN(path, path_len) \
    if (strlen(path) != path_len) { \
	RETURN_FALSE; \
    }
#  endif

#  define EIO_REGISTER_LONG_EIO_CONSTANT(name) \
    REGISTER_LONG_CONSTANT(#name, name, \
	    CONST_CS | CONST_PERSISTENT);

#  define EIO_REGISTER_LONG_CONSTANT(name, value) \
    REGISTER_LONG_CONSTANT(#name, value, \
	    CONST_CS | CONST_PERSISTENT);

#  define EIO_CB_CUSTOM_IS_LOCKED(eio_cb) ((eio_cb) ? (eio_cb)->locked : 0)

/* }}} */

/* {{{ Types */

typedef struct {
    zend_fcall_info *fci;
    zend_fcall_info_cache *fcc;
    zval* arg;					/* Arg for callback */
#  ifdef ZTS
    /* Thread context; to get rid of calling TSRMLS_FETCH() which consumes 
     * considerable amount of resources */
    void ***thread_ctx;
#  endif
} php_eio_cb_t;

typedef struct {
    zend_fcall_info *fci;
    zend_fcall_info_cache *fcc;
    zval* arg;					/* Arg for callback */
#  ifdef ZTS
    /* Thread context; to get rid of calling TSRMLS_FETCH() which consumes
     * considerable amount of resources */
    void ***thread_ctx;
#  endif
    zend_fcall_info *fci_exec;
    zend_fcall_info_cache *fcc_exec;
	zend_bool locked;
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
