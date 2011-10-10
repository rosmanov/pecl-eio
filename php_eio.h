/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyrght (C) 2011 Ruslan Osmanov <rrosmanov@gmail.com>               |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Ruslan Osmanov <rrosmanov@gmail.com>                         |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_EIO_H
#define PHP_EIO_H

extern zend_module_entry eio_module_entry;
#define phpext_eio_ptr &eio_module_entry

#ifdef PHP_WIN32
#	define PHP_EIO_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_EIO_API __attribute__ ((visibility("default")))
#else
#	define PHP_EIO_API
#endif

#ifndef PHP_EIO_VERSION
#define PHP_EIO_VERSION "0.0.1dev"
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#if !defined(EIO_NON_LINUX) && (defined(PHP_WIN32) || \
		defined(__BEOS__) || defined(NETWARE))
#define EIO_NON_LINUX	1
#endif

PHP_MINIT_FUNCTION(eio);
PHP_MSHUTDOWN_FUNCTION(eio);
PHP_RINIT_FUNCTION(eio);
PHP_RSHUTDOWN_FUNCTION(eio);
PHP_MINFO_FUNCTION(eio);

PHP_FUNCTION(eio_poll);
PHP_FUNCTION(eio_event_loop);

PHP_FUNCTION(eio_open);
PHP_FUNCTION(eio_truncate);
PHP_FUNCTION(eio_chown);
PHP_FUNCTION(eio_chmod);
PHP_FUNCTION(eio_mkdir);
PHP_FUNCTION(eio_rmdir);
PHP_FUNCTION(eio_unlink);
PHP_FUNCTION(eio_utime);
PHP_FUNCTION(eio_mknod);
PHP_FUNCTION(eio_link);
PHP_FUNCTION(eio_symlink);
PHP_FUNCTION(eio_rename);
PHP_FUNCTION(eio_close);
PHP_FUNCTION(eio_sync);
PHP_FUNCTION(eio_fsync);
PHP_FUNCTION(eio_fdatasync);
PHP_FUNCTION(eio_futime);
PHP_FUNCTION(eio_ftruncate);
PHP_FUNCTION(eio_fchmod);
PHP_FUNCTION(eio_fchown);
PHP_FUNCTION(eio_dup2);

PHP_FUNCTION(eio_read);
PHP_FUNCTION(eio_write);

PHP_FUNCTION(eio_readlink);
PHP_FUNCTION(eio_realpath);
PHP_FUNCTION(eio_stat);
PHP_FUNCTION(eio_lstat);
PHP_FUNCTION(eio_fstat);
PHP_FUNCTION(eio_statvfs);
PHP_FUNCTION(eio_fstatvfs);

PHP_FUNCTION(eio_readdir);

#ifndef EIO_NON_LINUX
PHP_FUNCTION(eio_sendfile);
PHP_FUNCTION(eio_readahead);
PHP_FUNCTION(eio_syncfs);
PHP_FUNCTION(eio_sync_file_range);
PHP_FUNCTION(eio_fallocate);
#endif

PHP_FUNCTION(eio_custom);
PHP_FUNCTION(eio_busy);
PHP_FUNCTION(eio_nop);

PHP_FUNCTION(eio_cancel);

PHP_FUNCTION(eio_grp);
PHP_FUNCTION(eio_grp_add);
PHP_FUNCTION(eio_grp_cancel);
PHP_FUNCTION(eio_grp_limit);

PHP_FUNCTION(eio_set_max_poll_time);
PHP_FUNCTION(eio_set_max_poll_reqs);
PHP_FUNCTION(eio_set_min_parallel);
PHP_FUNCTION(eio_set_max_parallel);
PHP_FUNCTION(eio_set_max_idle);
PHP_FUNCTION(eio_nthreads);
PHP_FUNCTION(eio_nreqs);
PHP_FUNCTION(eio_nready);
PHP_FUNCTION(eio_npending);

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)

#define PHP_EIO_GRP_DESCRIPTOR_NAME "EIO Group Descriptor"
#define PHP_EIO_REQ_DESCRIPTOR_NAME "EIO Request Descriptor"

#ifndef PHP_EIO_SHM_KEY_
#define PHP_EIO_SHM_KEY_ "/tmp/php-eio-shm"
#endif

#define PHP_EIO_SHM_KEY QUOTEME(PHP_EIO_SHM_KEY_)

#ifndef PHP_EIO_SHM_PERM
#define PHP_EIO_SHM_PERM 0600
#endif

/*
   ZEND_BEGIN_MODULE_GLOBALS(eio)
   ZEND_END_MODULE_GLOBALS(eio)
*/

/* In every utility function you add that needs to use variables 
   in php_eio_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as EIO_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

/*
#ifdef ZTS
#define EIO_G(v) TSRMG(eio_globals_id, zend_eio_globals *, v)
#else
#define EIO_G(v) (eio_globals.v)
#endif
*/

/* {{{ Macros */

#ifdef ZTS
#define TSRMLS_FETCH_FROM_CTX(ctx) void ***tsrm_ls = (void ***) ctx
#define TSRMLS_SET_CTX(ctx) ctx = (void ***) tsrm_ls
#else
#define TSRMLS_FETCH_FROM_CTX(ctx)
#define TSRMLS_SET_CTX(ctx)
#endif



#define EIO_RET_IF_NOT_CALLABLE(callback) 						\
	char *func_name; 												\
	if (callback && Z_TYPE_P(callback) != IS_NULL) {				\
		if (!zend_is_callable(callback, 0, &func_name TSRMLS_CC)) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, 			\
					"'%s' is not a valid callback", func_name);		\
			efree(func_name); 										\
			RETURN_FALSE; 											\
		} 															\
		efree(func_name); 											\
	}

#ifdef EIO_DEBUG
#define EIO_RET_IF_FAILED(req, eio_func) 						\
	if (!req || req->result != 0) { 								\
		php_error_docref(NULL TSRMLS_CC, E_ERROR, 					\
			#eio_func " failed, errno: %d", errno);					\
		RETURN_FALSE;												\
	}
#else
#define EIO_RET_IF_FAILED(req, eio_func) 						\
	if (!req || req->result != 0) 									\
		RETURN_FALSE;
#endif

#define EIO_RET_REQ_RESOURCE(req, eio_func) 						\
	EIO_RET_IF_FAILED(req, eio_func);							\
	ZEND_REGISTER_RESOURCE(return_value, req, le_eio_req);

#define EIO_INIT_CUSTOM(pri, callback, data, execute, eio_cb, req)	\
	long pri = EIO_PRI_DEFAULT;											\
	zval *callback = NULL, *data = NULL, *execute = NULL;				\
	php_eio_cb_custom_t *eio_cb;										\
	eio_req *req;														\
	/*php_eio_init(TSRMLS_C);*/

#define EIO_INIT(pri, callback, data, eio_cb, req)				\
	long pri = EIO_PRI_DEFAULT;										\
	zval *callback = NULL, *data = NULL;							\
	php_eio_cb_t *eio_cb;										\
	eio_req *req;													\
	/*php_eio_init(TSRMLS_C);*/

#define EIO_NEW_CB(eio_cb, callback, data) 						\
	eio_cb = php_eio_new_eio_cb(callback, data TSRMLS_CC);

#ifdef EIO_DEBUG
#define EIO_CHECK_PATH_LEN(path, path_len)						\
	if (strlen(path) != path_len) {									\
		php_error_docref(NULL TSRMLS_CC, E_WARNING, 				\
				"failed calculating path length");					\
		RETURN_FALSE;												\
	}
#else
#define EIO_CHECK_PATH_LEN(path, path_len)						\
	if (strlen(path) != path_len) {									\
		RETURN_FALSE;												\
	}
#endif

#ifdef EIO_DEBUG
#define EIO_CHECK_WRITABLE(path, file)							\
    /*
	 *if (access(path, W_OK) != 0) {									\
	 *    php_error_docref(NULL TSRMLS_CC, E_NOTICE, 					\
	 *    #file " '%s' is not writable", path);						\
	 *    errno = EACCES;												\
	 *    RETURN_FALSE;												\
	 *}																
     */
#else			
#define EIO_CHECK_WRITABLE(path, file)							\
    /*
	 *if (access(path, W_OK) != 0) {									\
	 *    errno = EACCES;												\
	 *    RETURN_FALSE;												\
	 *}																
     */
#endif

#ifdef EIO_DEBUG
#define EIO_CHECK_READABLE(path, file)								\
    /*
	 *if (access(path, W_OK) != 0) {								\
	 *    php_error_docref(NULL TSRMLS_CC, E_NOTICE, 				\
	 *    #file " '%s' is not readable", path);						\
	 *    errno = EACCES;											\
	 *    RETURN_FALSE;												\
	 *}																
     */
#else			
#define EIO_CHECK_READABLE(path, file)								\
    /*
	 *if (access(path, W_OK) != 0) {								\
	 *    errno = EACCES;											\
	 *    RETURN_FALSE;												\
	 *}																
     */
#endif

#define EIO_EVENT_LOOP()											\
	int semid;														\
	semid = php_eio_bin_semaphore_get(PHP_EIO_SHM_KEY, 0);			\
	while (eio_nreqs ())											\
	{																\
		/* Is want_poll() needed then? */							\
		php_eio_bin_semaphore_poll(semid);							\
		eio_poll();													\
	}


#define EIO_REGISTER_LONG_EIO_CONSTANT(name)						\
	REGISTER_LONG_CONSTANT(#name, name, 							\
			CONST_CS | CONST_PERSISTENT);

#define EIO_REGISTER_LONG_CONSTANT(name, value)						\
	REGISTER_LONG_CONSTANT(#name, value, 							\
			CONST_CS | CONST_PERSISTENT);

/* }}} */

/* {{{ Types */
#ifdef ZTS
#define EIO_CB_T_COMMON 														\
	zval* func;		/* Userspace callback */									\
	zval* arg;		/* Argument for the userspace callback */					\
	/* Thread context; to get rid of calling TSRMLS_FETCH() which consumes		\
	 * considerable amount of resources */										\
	void ***thread_ctx; 														
#else
#define EIO_CB_T_COMMON 														\
	zval* func;																	\
	zval* arg;								
#endif

typedef union php_eio_semun_ {
	int val; 
	struct semid_ds *buf; 
	unsigned short int *array;
	struct seminfo *__buf;
} php_eio_semun_t;

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

#endif	/* PHP_EIO_H */

/* vim: ft=h.c fdm=marker 
 */
