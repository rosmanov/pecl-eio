/*
  +----------------------------------------------------------------------+
  | PHP Version 5														 |
  +----------------------------------------------------------------------+
  | Copyrght (C) 2011 Ruslan Osmanov <osmanov@php.net>					 |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,		 |
  | that is bundled with this package in the file LICENSE, and is		 |
  | available through the world-wide-web at the following url:			 |
  | http://www.php.net/license/3_01.txt									 |
  | If you did not receive a copy of the PHP license and are unable to	 |
  | obtain it through the world-wide-web, please send a note to			 |
  | license@php.net so we can mail you a copy immediately.				 |
  +----------------------------------------------------------------------+
  | Author: Ruslan Osmanov <osmanov@php.net>							 |
  +----------------------------------------------------------------------+
*/

#ifndef EIO_PRIV_H
#define EIO_PRIV_H

extern const zend_function_entry eio_functions[];

#define PHP_EIO_GRP_DESCRIPTOR_NAME "EIO Group Descriptor"
#define PHP_EIO_REQ_DESCRIPTOR_NAME "EIO Request Descriptor"

/* {{{ Macros */

#ifdef ZTS
#define TSRMLS_FETCH_FROM_CTX(ctx) void ***tsrm_ls = (void ***) ctx
#define TSRMLS_SET_CTX(ctx) ctx = (void ***) tsrm_ls
#else
#define TSRMLS_FETCH_FROM_CTX(ctx)
#define TSRMLS_SET_CTX(ctx)
#endif


# define PHP_EIO_EVENTFD_INIT php_eio_eventfd = eventfd(0, EFD_NONBLOCK);
# define PHP_EIO_EVENTFD_DESTROY 			\
	if (php_eio_eventfd) {					\
		close(php_eio_eventfd);				\
	}
# define PHP_EIO_EVENTFD_WRITE						\
	uint64_t u = 1;									\
	write(php_eio_eventfd, &u, sizeof(uint64_t));
# define PHP_EIO_EVENTFD_READ						\
	uint64_t u;										\
	read(php_eio_eventfd, &u, sizeof(uint64_t));

#define EIO_EVENT_LOOP						\
	while (eio_nreqs()) {					\
		struct pollfd pfd;					\
		pfd.fd = php_eio_eventfd;			\
		pfd.events = POLLIN;				\
		poll (&pfd, 1, -1);					\
		eio_poll();							\
	}	


#define EIO_RET_IF_NOT_CALLABLE(callback)							\
	char *func_name;												\
	if (callback && Z_TYPE_P(callback) != IS_NULL) {				\
		if (!zend_is_callable(callback, 0, &func_name TSRMLS_CC)) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING,				\
					"'%s' is not a valid callback", func_name);		\
			efree(func_name);										\
			RETURN_FALSE;											\
		}															\
		efree(func_name);											\
	}

#ifdef EIO_DEBUG
# define EIO_RET_IF_FAILED(req, eio_func)							\
	if (!req || req->result != 0) {									\
		php_error_docref(NULL TSRMLS_CC, E_ERROR,					\
			#eio_func " failed: %s", strerror(req->errorno));		\
		RETURN_FALSE;												\
	}
#else
# define EIO_RET_IF_FAILED(req, eio_func)							\
	if (!req || req->result != 0)									\
		RETURN_FALSE;
#endif

#define EIO_RET_REQ_RESOURCE(req, eio_func)							\
	EIO_RET_IF_FAILED(req, eio_func);								\
	ZEND_REGISTER_RESOURCE(return_value, req, le_eio_req);

#define EIO_INIT_CUSTOM(pri, callback, data, execute, eio_cb, req)	\
	long pri = EIO_PRI_DEFAULT;										\
	zval *callback = NULL, *data = NULL, *execute = NULL;			\
	php_eio_cb_custom_t *eio_cb;									\
	eio_req *req;													\
	/*php_eio_init(TSRMLS_C);*/

#define EIO_INIT(pri, callback, data, eio_cb, req)					\
	long pri = EIO_PRI_DEFAULT;										\
	zval *callback = NULL, *data = NULL;							\
	php_eio_cb_t *eio_cb;											\
	eio_req *req;													\
	/*php_eio_init(TSRMLS_C);*/

#define EIO_NEW_CB(eio_cb, callback, data)							\
	eio_cb = php_eio_new_eio_cb(callback, data TSRMLS_CC);

#ifdef EIO_DEBUG
#define EIO_CHECK_PATH_LEN(path, path_len)							\
	if (strlen(path) != path_len) {									\
		php_error_docref(NULL TSRMLS_CC, E_WARNING,					\
				"failed calculating path length");					\
		RETURN_FALSE;												\
	}
#else
#define EIO_CHECK_PATH_LEN(path, path_len)							\
	if (strlen(path) != path_len) {									\
		RETURN_FALSE;												\
	}
#endif


#define EIO_REGISTER_LONG_EIO_CONSTANT(name)						\
	REGISTER_LONG_CONSTANT(#name, name,								\
			CONST_CS | CONST_PERSISTENT);

#define EIO_REGISTER_LONG_CONSTANT(name, value)						\
	REGISTER_LONG_CONSTANT(#name, value,							\
			CONST_CS | CONST_PERSISTENT);

#define EIO_CB_CUSTOM_LOCK eio_cb->locked = 1;
#define EIO_CB_CUSTOM_UNLOCK eio_cb->locked = 0;
#define EIO_CB_CUSTOM_IS_LOCKED(eio_cb) ((eio_cb) ? (eio_cb)->locked : 0)

/* }}} */

/* {{{ Types */
#ifdef ZTS
#define EIO_CB_T_COMMON															\
	zval* func;		/* Userspace callback */									\
	zval* arg;		/* Argument for the userspace callback */					\
	/* Thread context; to get rid of calling TSRMLS_FETCH() which consumes		\
	 * considerable amount of resources */										\
	void ***thread_ctx;															
#else
#define EIO_CB_T_COMMON															\
	zval* func;																	\
	zval* arg;								
#endif

typedef struct php_eio_cb_ {
	EIO_CB_T_COMMON;
} php_eio_cb_t;

typedef struct php_eio_cb_custom_ {
	EIO_CB_T_COMMON;
	zval* exec;
	zend_bool locked;
} php_eio_cb_custom_t;

#undef EIO_CB_T_COMMON
/* }}} */

#endif	/* EIO_PRIV_H */
/* 
 * vim: ft=h.c fdm=marker 
 */
