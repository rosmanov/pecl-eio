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
  | Notes																 |
  |																		 |
  | eio_mlock(), eio_mlockall(), eio_msync(), eio_mtouch() are not		 |
  | implemented in this													 |
  | extension because of PHP's obvious limitations on user-sie memory	 |
  | management.															 |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h> /* strerror() */
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#ifndef EIO_NON_LINUX
#include "linux/falloc.h"
#endif

#include "php_eio.h"

#include "eio.h"

static int le_eio_grp;
static int le_eio_req;

/* {{{ ARG_INFO */
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION > 5
# define EIO_ARGINFO
#else
# define EIO_ARGINFO static
#endif

#define EIO_ARGINFO_FUNC_0(name)							\
EIO_ARGINFO													\
ZEND_BEGIN_ARG_INFO(arginfo_eio_##name, 0)					\
ZEND_END_ARG_INFO()

#define EIO_ARGINFO_FUNC_1(name, arg)						\
EIO_ARGINFO													\
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_##name, 0, 0, 1)			\
	ZEND_ARG_INFO(0, arg)									\
ZEND_END_ARG_INFO()

#define EIO_ARGINFO_PRI_CB_DATA								\
	ZEND_ARG_INFO(0, pri)									\
	ZEND_ARG_INFO(0, callback)								\
	ZEND_ARG_INFO(0, data)								

#define EIO_ARGINFO_FUNC_1_N(name, arg, n)					\
EIO_ARGINFO													\
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_##name, 0, 0, n)			\
	ZEND_ARG_INFO(0, arg)									\
	EIO_ARGINFO_PRI_CB_DATA									\
ZEND_END_ARG_INFO()

EIO_ARGINFO_FUNC_0(poll);
EIO_ARGINFO_FUNC_0(event_loop);

/* {{{ POSIX API WRAPPERS */
EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_open, 0, 0, 5)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, mode)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_truncate, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, offset)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_chown, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, uid)
	ZEND_ARG_INFO(0, gid)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_chmod, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, mode)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_mkdir, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, mode)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO_FUNC_1_N(rmdir, path, 1)

EIO_ARGINFO_FUNC_1_N(unlink, path, 1)

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_utime, 0, 0, 3)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, atime)
	ZEND_ARG_INFO(0, mtime)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_mknod, 0, 0, 3)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, dev)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_link, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, new_path)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_symlink, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, new_path)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_rename, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, new_path)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO_FUNC_1_N(close, fd, 1)

EIO_ARGINFO_FUNC_0(sync)

EIO_ARGINFO_FUNC_1_N(fsync, fd, 1)

EIO_ARGINFO_FUNC_1_N(fdatasync, fd, 1)

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_futime, 0, 0, 3)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, atime)
	ZEND_ARG_INFO(0, mtime)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()
	
EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_ftruncate, 0, 0, 2)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, offset)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_fchmod, 0, 0, 2)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, mode)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_fchown, 0, 0, 2)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, uid)
	ZEND_ARG_INFO(0, gid)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_dup2, 0, 0, 2)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, fd2)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_read, 0, 0, 5)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, offset)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_write, 0, 0, 2)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(1, str)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, offset)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO_FUNC_1_N(readlink, path, 3)
EIO_ARGINFO_FUNC_1_N(realpath, path, 3)
EIO_ARGINFO_FUNC_1_N(stat, path, 3)
EIO_ARGINFO_FUNC_1_N(lstat, path, 3)
EIO_ARGINFO_FUNC_1_N(fstat, fd, 3)
EIO_ARGINFO_FUNC_1_N(statvfs, path, 3)
EIO_ARGINFO_FUNC_1_N(fstatvfs, fd, 3)
/* }}} */

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_readdir, 0, 0, 4)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, flags)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()


/* {{{ if not EIO_NON_LINUX */
#ifndef EIO_NON_LINUX
EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_sendfile, 0, 0, 4)
	ZEND_ARG_INFO(0, out_fd)
	ZEND_ARG_INFO(0, in_fd)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, pri)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_readahead, 0, 0, 3)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, pri)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_syncfs, 0, 0, 1)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, pri)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_sync_file_range, 0, 0, 4)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, nbytes)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, pri)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()
#endif
/* }}} */

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_fallocate, 0, 0, 4)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, length)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO_FUNC_1_N(custom, execute, 3)

EIO_ARGINFO_FUNC_1_N(busy, delay, 1)

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_nop, 0, 0, 0)
	EIO_ARGINFO_PRI_CB_DATA
ZEND_END_ARG_INFO()

EIO_ARGINFO												
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_cancel, 0, 0, 1)
	ZEND_ARG_INFO(0, req)
ZEND_END_ARG_INFO()

/* {{{ GROUPING AND LIMITING */
EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_grp, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_grp_add, 0, 0, 2)
	ZEND_ARG_INFO(0, grp)
	ZEND_ARG_INFO(0, req)
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_grp_cancel, 0, 0, 1)
	ZEND_ARG_INFO(0, grp)
ZEND_END_ARG_INFO()

EIO_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_grp_limit, 0, 0, 2)
	ZEND_ARG_INFO(0, grp)
	ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ CONFIGURATION */

EIO_ARGINFO_FUNC_1(set_max_poll_time, nseconds)
EIO_ARGINFO_FUNC_1(set_max_poll_reqs, nreqs)
EIO_ARGINFO_FUNC_1(set_min_parallel, nthreads)
EIO_ARGINFO_FUNC_1(set_max_parallel, nthreads)
EIO_ARGINFO_FUNC_1(set_max_idle, nthreads)

EIO_ARGINFO_FUNC_0(nthreads)
EIO_ARGINFO_FUNC_0(nreqs)
EIO_ARGINFO_FUNC_0(nready)
EIO_ARGINFO_FUNC_0(npending)

/* }}} */

#undef EIO_ARGINFO
#undef EIO_ARGINFO_FUNC_0
#undef EIO_ARGINFO_PRI_CB_DATA
/* }}} */

/* {{{ eio_functions[]
 *
 * Every user visible function must have an entry in eio_functions[].
 */
const zend_function_entry eio_functions[] = {
	PHP_FE(eio_poll, arginfo_eio_poll)
	PHP_FE(eio_event_loop, arginfo_eio_event_loop)
	PHP_FE(eio_open, arginfo_eio_open)
	PHP_FE(eio_truncate, arginfo_eio_truncate)
	PHP_FE(eio_chown, arginfo_eio_chown)
	PHP_FE(eio_chmod, arginfo_eio_chmod)
	PHP_FE(eio_mkdir, arginfo_eio_mkdir)
	PHP_FE(eio_rmdir, arginfo_eio_rmdir)
	PHP_FE(eio_unlink, arginfo_eio_unlink)
	PHP_FE(eio_utime, arginfo_eio_utime)
	PHP_FE(eio_mknod, arginfo_eio_mknod)
	PHP_FE(eio_link, arginfo_eio_link)
	PHP_FE(eio_symlink, arginfo_eio_symlink)
	PHP_FE(eio_rename, arginfo_eio_rename)
	PHP_FE(eio_close, arginfo_eio_close)
	PHP_FE(eio_sync, arginfo_eio_sync)
	PHP_FE(eio_fsync, arginfo_eio_fsync)
	PHP_FE(eio_fdatasync, arginfo_eio_fdatasync)
	PHP_FE(eio_futime, arginfo_eio_futime)
	PHP_FE(eio_ftruncate, arginfo_eio_ftruncate)
	PHP_FE(eio_fchmod, arginfo_eio_fchmod)
	PHP_FE(eio_fchown, arginfo_eio_fchown)
	PHP_FE(eio_dup2, arginfo_eio_dup2)
	PHP_FE(eio_read, arginfo_eio_read)
	PHP_FE(eio_write, arginfo_eio_write)
	PHP_FE(eio_readlink, arginfo_eio_readlink)
	PHP_FE(eio_realpath, arginfo_eio_realpath)
	PHP_FE(eio_stat, arginfo_eio_stat)
	PHP_FE(eio_lstat, arginfo_eio_lstat)
	PHP_FE(eio_fstat, arginfo_eio_fstat)
	PHP_FE(eio_statvfs, arginfo_eio_statvfs)
	PHP_FE(eio_fstatvfs, arginfo_eio_fstatvfs)
	PHP_FE(eio_readdir, arginfo_eio_readdir)
#ifndef EIO_NON_LINUX
	PHP_FE(eio_sendfile, arginfo_eio_sendfile)
	PHP_FE(eio_readahead, arginfo_eio_readahead)
	PHP_FE(eio_syncfs, arginfo_eio_syncfs)
	PHP_FE(eio_sync_file_range, arginfo_eio_sync_file_range)
	PHP_FE(eio_fallocate, arginfo_eio_fallocate)
#endif
	PHP_FE(eio_custom, arginfo_eio_custom)
	PHP_FE(eio_busy, arginfo_eio_busy)
	PHP_FE(eio_nop, arginfo_eio_nop)
	PHP_FE(eio_cancel, arginfo_eio_cancel)
	PHP_FE(eio_grp, arginfo_eio_grp)
	PHP_FE(eio_grp_add, arginfo_eio_grp_add)
	PHP_FE(eio_grp_cancel, arginfo_eio_grp_cancel)
	PHP_FE(eio_grp_limit, arginfo_eio_grp_limit)
	PHP_FE(eio_set_max_poll_time, arginfo_eio_set_max_poll_time)

	PHP_FE(eio_set_max_poll_reqs, arginfo_eio_set_max_poll_reqs)
	PHP_FE(eio_set_min_parallel, arginfo_eio_set_min_parallel)
	PHP_FE(eio_set_max_parallel, arginfo_eio_set_max_parallel)
	PHP_FE(eio_set_max_idle, arginfo_eio_set_max_idle)
	PHP_FE(eio_nthreads, arginfo_eio_nthreads)
	PHP_FE(eio_nreqs, arginfo_eio_nreqs)
	PHP_FE(eio_nready, arginfo_eio_nready)
	PHP_FE(eio_npending, arginfo_eio_npending)
	{NULL, NULL, NULL}	/* Must be the last line in eio_functions[] */
};
/* }}} */

static const zend_module_dep eio_deps[] = { /* {{{ */
	/* TODO: Replace with ZEND_MOD_CONFLICTS_EX("xdebug", rel, ver) when the
	 * issue will be fixed: http://bugs.xdebug.org/view.php?id=725 */
	ZEND_MOD_CONFLICTS("xdebug")		
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ eio_module_entry
 */
zend_module_entry eio_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	eio_deps,
	"eio",
	eio_functions,
	PHP_MINIT(eio),
	PHP_MSHUTDOWN(eio),
	PHP_RINIT(eio),
	PHP_RSHUTDOWN(eio),
	PHP_MINFO(eio),
	PHP_EIO_VERSION,	
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_EIO
ZEND_GET_MODULE(eio)
#endif


/* {{{ Internal functions */

#define EIO_CB_ALLOC(type) type *eio_cb = ecalloc(1, sizeof(type));

#define EIO_CB_SET_FIELD(n, v)		\
	if (v) {						\
		zval_add_ref(&v);			\
	} else {						\
		ALLOC_INIT_ZVAL(v);			\
	}								\
	eio_cb->n= v;

#define EIO_REQ_TYPE(type)	(type)

#define EIO_REQ_WARN_RESULT_ERROR()										\
	php_error_docref(NULL TSRMLS_CC, E_WARNING,							\
			"%s, eio_req result: %ld, req type: %s",					\
			strerror(req->errorno), (long) EIO_RESULT(req), 			\
				php_eio_get_req_type_str(req->type))

#define EIO_REQ_WARN_INVALID_CB()										\
	php_error_docref(NULL TSRMLS_CC, E_WARNING,							\
			"'%s' is not a valid callback", func_name);

#define EIO_REQ_CB_INIT(cb_type)										\
	cb_type *eio_cb = (cb_type*) req->data;								\
	TSRMLS_FETCH_FROM_CTX(eio_cb ? eio_cb->thread_ctx : NULL);			\
	zval retval;														\
	char *func_name; 

#define EIO_REQ_FREE_ARGS					\
		zval_ptr_dtor(&(args[0]));			\
		zval_ptr_dtor(&(args[1]));

#define EIO_BUF_ZVAL_P(req) ((zval *)(EIO_BUF(req)))

#define EIO_BUF_FETCH_FROM_ZVAL(req, z)		\
	EIO_BUF(req) = emalloc(sizeof(zval));	\
	*EIO_BUF_ZVAL_P(req) = z;				\
	zval_copy_ctor( EIO_BUF_ZVAL_P(req) );

#define EIO_CB_CUSTOM_LOCK eio_cb->locked = 1;
#define EIO_CB_CUSTOM_UNLOCK eio_cb->locked = 0;
#define EIO_CB_CUSTOM_IS_LOCKED(eio_cb) ((eio_cb) ? (eio_cb)->locked : 0)

/* {{{ php_eio_get_req_type_str */
#define CASE_RET_STR(x) case x: return #x
static const char *
php_eio_get_req_type_str(unsigned int code) 
{
	switch(code) {
		CASE_RET_STR(EIO_CUSTOM);
		CASE_RET_STR(EIO_WD_OPEN);
		CASE_RET_STR(EIO_WD_CLOSE);
		CASE_RET_STR(EIO_CLOSE);
		CASE_RET_STR(EIO_DUP2);
		CASE_RET_STR(EIO_READ);
		CASE_RET_STR(EIO_WRITE);
		CASE_RET_STR(EIO_READAHEAD);
		CASE_RET_STR(EIO_SENDFILE);
		CASE_RET_STR(EIO_FSTAT);
		CASE_RET_STR(EIO_FSTATVFS);
		CASE_RET_STR(EIO_FTRUNCATE);
		CASE_RET_STR(EIO_FUTIME);
		CASE_RET_STR(EIO_FCHMOD);
		CASE_RET_STR(EIO_FCHOWN);
		CASE_RET_STR(EIO_SYNC);
		CASE_RET_STR(EIO_FSYNC);
		CASE_RET_STR(EIO_FDATASYNC);
		CASE_RET_STR(EIO_SYNCFS);
		CASE_RET_STR(EIO_MSYNC);
		CASE_RET_STR(EIO_MTOUCH);
		CASE_RET_STR(EIO_SYNC_FILE_RANGE);
		CASE_RET_STR(EIO_FALLOCATE);
		CASE_RET_STR(EIO_MLOCK);
		CASE_RET_STR(EIO_MLOCKALL);
		CASE_RET_STR(EIO_GROUP);
		CASE_RET_STR(EIO_NOP);
		CASE_RET_STR(EIO_BUSY);
		CASE_RET_STR(EIO_REALPATH);
		CASE_RET_STR(EIO_STATVFS);
		CASE_RET_STR(EIO_READDIR);
		CASE_RET_STR(EIO_OPEN);
		CASE_RET_STR(EIO_STAT);
		CASE_RET_STR(EIO_LSTAT);
		CASE_RET_STR(EIO_TRUNCATE);
		CASE_RET_STR(EIO_UTIME);
		CASE_RET_STR(EIO_CHMOD);
		CASE_RET_STR(EIO_CHOWN);
		CASE_RET_STR(EIO_UNLINK);
		CASE_RET_STR(EIO_RMDIR);
		CASE_RET_STR(EIO_MKDIR);
		CASE_RET_STR(EIO_RENAME);
		CASE_RET_STR(EIO_MKNOD);
		CASE_RET_STR(EIO_LINK);
		CASE_RET_STR(EIO_SYMLINK);
		CASE_RET_STR(EIO_READLINK);
		CASE_RET_STR(EIO_REQ_TYPE_NUM);
		default: return "UNKNOWN";
	}
}
#undef CASE_RET_STR
/* }}} */

/* Returns a binary semaphore's ID, allocate if nesessary */
static inline int
php_eio_bin_semaphore_get(const char *str_key, int sem_flags) 
{
	return semget(ftok(str_key, 'E'), 1, sem_flags);
}

static inline int 
php_eio_bin_semaphore_dealloc(int semid)
{
	php_eio_semun_t s;

	return semctl(semid, 1, IPC_RMID, s);
}

/* Init semaphore with value of 1 */
static inline int 
php_eio_bin_semaphore_init(int semid)
{
	php_eio_semun_t s; 
	unsigned short values[1]; 

	values[0] = 1;
	s.array = values; 

	return semctl(semid, 0, SETALL, s);
}

/* {{{ php_eio_bin_semaphore_wait
 * Wait on a binary semaphore. Will *NOT* Block until semaphore value is
 * positive (IPC_NOWAIT) , then decrement it by 1 */
static inline int
php_eio_bin_semaphore_wait(int semid)
{
	struct sembuf operations[1]; 

	operations[0].sem_num = 0;
	/* Decrement by 1*/
	operations[0].sem_op = -1;
	/* Permit undo'ing */
	/* Prevent blocking, since we poll in event loop */
	operations[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

	return semop(semid, operations, 1);
}
/* }}} */

/* {{{ php_eio_bin_semaphore_poll 
 * Wait on a binary semaphore. *Will* block until semaphore value is
 * positive , then decrement it by 1 */
static inline int
php_eio_bin_semaphore_poll(int semid)
{
	struct sembuf operations[1]; 

	operations[0].sem_num = 0;
	/* Decrement by 1*/
	operations[0].sem_op = -1;
	/* Permit undo'ing */
	operations[0].sem_flg = SEM_UNDO;

	return semop(semid, operations, 1);
}
/* }}} */

/* {{{ php_eio_bin_semaphore_post
 * Post to a binary semaphore: increment it's value by 1.
 * This returns immediately */
static inline int
php_eio_bin_semaphore_post(int semid)
{
	struct sembuf operations[1]; 

	operations[0].sem_num = 0;
	/* Decrement by 1*/
	operations[0].sem_op = 1;
	/* Permit undo'ing */
	/* Probably, IPC_NOWAIT is not required here */
	operations[0].sem_flg = SEM_UNDO | IPC_NOWAIT; 

	return semop(semid, operations, 1);
}
/* }}} */

/* {{{ php_eio_set_readdir_names */
static void
php_eio_set_readdir_names(zval *z, const eio_req *req)
{
	int i, len;
	zval *names_array;
	char *names = EIO_BUF(req);

	MAKE_STD_ZVAL(names_array);
	array_init(names_array);

	for (i = 0; i < EIO_RESULT(req); ++i)
	{
		/* Not good idea to use strlen(). eio uses it internally in
		 * eio__scandir() though. */
		len = strlen(names);
		add_index_stringl(names_array, i, names, len, 1);

		/* move to next name */
		names += len;
		names++;
	}
	add_assoc_zval(z, "names", names_array);
}
/* }}} */

/* {{{ php_eio_set_readdir_dents 
*/
static void
php_eio_set_readdir_dent_and_names(zval *z, const eio_req *req) 
{
	zval *names_array, *dents_array;
	int i;
	char *names = EIO_BUF(req);
	struct eio_dirent *ents = (struct eio_dirent *)req->ptr1;

	/* $names_array = array(); 
	 * $dents_array = array(); */
	MAKE_STD_ZVAL(names_array);
	MAKE_STD_ZVAL(dents_array);
	array_init(names_array);
	array_init(dents_array);

	for (i = 0; i < EIO_RESULT(req); ++i)
	{
		struct eio_dirent *ent = ents + i;
		char *name = names + ent->nameofs;

		/* $names_array[i] = name */
		add_index_stringl(names_array, i, name, ent->namelen, 1);

		/* $ent_array = array() */
		zval *ent_array; 
		MAKE_STD_ZVAL(ent_array);
		array_init(ent_array);

		/* $ent_array['name'] = ent->name */
		add_assoc_stringl(ent_array, "name", name, ent->namelen, 1);
		/* $ent_array['type'] = ent->type */
		add_assoc_long(ent_array, "type", ent->type);
		/* $ent_array['inode'] = ent->type */
		add_assoc_long(ent_array, "inode", ent->inode);

		/* $dents_array[i] = $ent_array */
		add_index_zval(dents_array, i, ent_array);
	}

	add_assoc_zval(z, "names", names_array);
	add_assoc_zval(z, "dents", dents_array);
}
/* }}} */

/* {{{ php_eio_free_eio_cb 
 * Free an instance of php_eio_cb_t */
static inline void 
php_eio_free_eio_cb(php_eio_cb_t *eio_cb)
{
	if (eio_cb) {
		zval_ptr_dtor(&eio_cb->arg); 
		zval_ptr_dtor(&eio_cb->func);	
		efree(eio_cb);
		eio_cb = NULL;
	}
}
/* }}} */

/* {{{ php_eio_free_eio_cb_custom
 * Free an instance of php_eio_cb_custom_t */
static inline void 
php_eio_free_eio_cb_custom(php_eio_cb_custom_t *eio_cb)
{
	if (eio_cb) {
		zval_ptr_dtor(&eio_cb->arg); 
		zval_ptr_dtor(&eio_cb->func);	
		zval_ptr_dtor(&eio_cb->exec);	
		efree(eio_cb);
		eio_cb = NULL;
	}
}
/* }}} */

/* {{{ php_eio_new_eio_cb 
 * Allocates memory for a new instance of php_eio_cb_t. 
 * Returns pointer to the new instance */
static inline php_eio_cb_t * 
php_eio_new_eio_cb(zval *callback, zval *data TSRMLS_DC)
{
	EIO_CB_ALLOC(php_eio_cb_t);

	EIO_CB_SET_FIELD(func, callback);
	EIO_CB_SET_FIELD(arg, data);

	TSRMLS_SET_CTX(eio_cb->thread_ctx);

	return eio_cb;
}
/* }}} */

/* {{{ php_eio_new_eio_cb_custom 
 * Allocates memory for a new instance of php_eio_cb_custom_t
 * Returns pointer to the new instance */
static inline php_eio_cb_custom_t * 
php_eio_new_eio_cb_custom(zval *callback, zval *data, zval *execute TSRMLS_DC)
{
	EIO_CB_ALLOC(php_eio_cb_custom_t);

	EIO_CB_SET_FIELD(func, callback);
	EIO_CB_SET_FIELD(arg, data);
	EIO_CB_SET_FIELD(exec, execute);

	TSRMLS_SET_CTX(eio_cb->thread_ctx);

	eio_cb->locked = 0;

	return eio_cb;
}
/* }}} */

/* {{{ php_eio_custom_execute 
 * Is called by eio_custom(). Calls userspace function. */
static void 
php_eio_custom_execute(eio_req *req)
{
	zval *args[1];
	EIO_REQ_CB_INIT(php_eio_cb_custom_t);

	if (EIO_CANCELLED(req)) {
		php_eio_free_eio_cb_custom(eio_cb);
		return;
	}
	EIO_CB_CUSTOM_LOCK;

	EIO_RESULT(req) = -1;	

	if (eio_cb) {
		if (!zend_is_callable(eio_cb->exec, 0, &func_name TSRMLS_CC)) {
			EIO_REQ_WARN_INVALID_CB();
		} else {
			args[0] = eio_cb->arg;

			if (call_user_function(EG(function_table), NULL, eio_cb->exec,
						&retval, 1, args TSRMLS_CC) == SUCCESS) {
				EIO_BUF_FETCH_FROM_ZVAL(req, retval);
				EIO_RESULT(req) = 0;
			}

			/* Don't destroy eio_cb->arg, since it is needed further in
			 * php_eio_res_cb_custom()
			 * zval_ptr_dtor(eio_cb->arg); */
		}

		efree(func_name);
	}
}
/* }}} */

/* {{{ php_eio_res_cb_custom */
static int 
php_eio_res_cb_custom(eio_req *req) 
{
	zval *args[2];
	EIO_REQ_CB_INIT(php_eio_cb_custom_t);

	if (!EIO_CB_CUSTOM_IS_LOCKED(eio_cb) && EIO_CANCELLED(req)) {
		php_eio_free_eio_cb_custom(eio_cb);
		return 0;
	}

	if (eio_cb) {
		if (!zend_is_callable(eio_cb->func, 0, &func_name TSRMLS_CC)) {
			EIO_REQ_WARN_INVALID_CB();
		} else {
			args[0] = eio_cb->arg;
			ALLOC_INIT_ZVAL(args[1]);
			*args[1] = *((zval *)EIO_BUF(req));
			zval_copy_ctor(args[1]);

			if (call_user_function(EG(function_table), NULL, eio_cb->func,
						&retval, 2, args TSRMLS_CC) == SUCCESS) {
				zval_dtor(&retval);
			}

			/* 
			 * Should be freed in EIO_REQ_FREE_ARGS;
			 * zval_dtor( ((zval *)EIO_BUF(req)) );
			 */
			EIO_REQ_FREE_ARGS;
		}

		efree(func_name);

	}

	php_eio_free_eio_cb_custom(eio_cb);

	return 0;
}
/* }}} */

/* {{{ php_eio_res_cb
 *
 * Callback for each of eio_* PHP functions. Calls userspace callback with it's
 * custom data, if in the initial eio_* call corresponding data provided
 * Returns non-zero value on failure.
 *
 * The userspace callback should match proto:
 * void callback($data, $result); 
 * $data is user custom data passed in eio_*() call
 * $result contains eio_* function specific value. E.g. for eio_open() it contains file descriptor 
 */ 
static int
php_eio_res_cb(eio_req *req)
{
	zval *args[2];
	EIO_REQ_CB_INIT(php_eio_cb_t);

	if (EIO_CANCELLED(req)) {
		php_eio_free_eio_cb(eio_cb);
		return 0;
	}

	/* req->data should be of type (php_eio_cb_t *)
	 * eio_cb->func = user callback as zval pointer 
	 * eio_cb->arg = user variable to be passed to callback 
	 * EIO_RESULT(req), i.e. req->result, = return value of corresponding
	 * system call(mkdir, rmdir etc.)
	 */
	/* WARNING. If this callback returns nonzero, eio will stop processing
	 * results(in eio_poll), and will return the value to it's caller */

	if (EIO_RESULT(req) < 0) {
		EIO_REQ_WARN_RESULT_ERROR();
	}

	if (!eio_cb) {
		return 0;
	}

	/* Call user's callback */
	if (eio_cb->func && Z_TYPE_P(eio_cb->func) != IS_NULL) {
		/* set $data arg value */
		args[0] = eio_cb->arg;

		/* {{{ set $result arg value */
		MAKE_STD_ZVAL(args[1]);
		switch (req->type) {
			case EIO_READ:
				/* EIO_BUF(req) is the buffer with read contents 
				 * (size_t) req->size = length parameter value passed to eio_read()
				 * Since req is destroyed later, data stored in req should be
				 * duplicated */
				if (EIO_RESULT(req) != -1) {
					ZVAL_STRINGL(args[1], EIO_BUF(req), req->size, 1);
				} else {
					ZVAL_NULL(args[1]);
				}
				break;

			case EIO_READLINK: case EIO_REALPATH:
				/* EIO_BUF(req) is NOT-null-terminated string of result
				 * EIO_RESULT(req) is the length of the string */
				ZVAL_STRINGL(args[1], EIO_BUF(req), EIO_RESULT(req), 1);
				break;

			case EIO_STAT: case EIO_LSTAT: case EIO_FSTAT:/* {{{ */
				/* EIO_STAT_BUF(req) is ptr to EIO_STRUCT_STAT structure */
				array_init(args[1]);

				add_assoc_long(args[1], "st_dev", EIO_STAT_BUF(req)->st_dev);
				add_assoc_long(args[1], "st_ino", EIO_STAT_BUF(req)->st_ino);
				add_assoc_long(args[1], "st_mode", EIO_STAT_BUF(req)->st_mode);
				add_assoc_long(args[1], "st_nlink", EIO_STAT_BUF(req)->st_nlink);
				add_assoc_long(args[1], "st_uid", EIO_STAT_BUF(req)->st_uid);
				add_assoc_long(args[1], "st_gid", EIO_STAT_BUF(req)->st_gid);
#ifdef HAVE_ST_RDEV
				add_assoc_long(args[1], "st_rdev", EIO_STAT_BUF(req)->st_rdev);
#else
				add_assoc_long(args[1], "st_rdev", -1);
#endif

#ifdef HAVE_ST_BLKSIZE
				add_assoc_long(args[1], "st_blksize", EIO_STAT_BUF(req)->st_blksize);
#else
				add_assoc_long(args[1], "st_blksize", -1);
#endif
#ifdef HAVE_ST_BLOCKS
				add_assoc_long(args[1], "st_blocks", EIO_STAT_BUF(req)->st_blocks);
#else
				add_assoc_long(args[1], "st_blocks", -1);
#endif

				add_assoc_long(args[1], "st_atime", EIO_STAT_BUF(req)->st_atime);
				add_assoc_long(args[1], "st_mtime", EIO_STAT_BUF(req)->st_mtime);
				add_assoc_long(args[1], "st_ctime", EIO_STAT_BUF(req)->st_ctime);
				break;
				/* }}} */

			case EIO_STATVFS: case EIO_FSTATVFS:/* {{{ */
				/* EIO_STATVFS_BUF(req) is ptr to EIO_STRUCT_STATVFS structure */
				array_init(args[1]);
				
				add_assoc_long(args[1], "f_bsize", EIO_STATVFS_BUF(req)->f_bsize);
				add_assoc_long(args[1], "f_frsize", EIO_STATVFS_BUF(req)->f_frsize);
				add_assoc_long(args[1], "f_blocks", EIO_STATVFS_BUF(req)->f_blocks);
				add_assoc_long(args[1], "f_bfree", EIO_STATVFS_BUF(req)->f_bfree);
				add_assoc_long(args[1], "f_bavail", EIO_STATVFS_BUF(req)->f_bavail);
				add_assoc_long(args[1], "f_files", EIO_STATVFS_BUF(req)->f_files);
				add_assoc_long(args[1], "f_ffree", EIO_STATVFS_BUF(req)->f_ffree);
				add_assoc_long(args[1], "f_favail", EIO_STATVFS_BUF(req)->f_favail);
				add_assoc_long(args[1], "f_fsid", EIO_STATVFS_BUF(req)->f_fsid);
				add_assoc_long(args[1], "f_flag", EIO_STATVFS_BUF(req)->f_flag);
				add_assoc_long(args[1], "f_namemax", EIO_STATVFS_BUF(req)->f_namemax);
				break;
				/* }}} */

			case EIO_READDIR:
				/* EIO_READDIR_* flags are in req->int1 
				 *
				 * EIO_BUF(req), which is req->ptr2, contains null-terminated names 
				 * These will be stored in $result['names'] as a vector */
				array_init(args[1]);

				if (req->int1 & (EIO_READDIR_DENTS | EIO_READDIR_DIRS_FIRST)) {
					/* fill $result['dents'] with array of struct eio_dirent like arrays 
					 * fill $result['names'] with dir names */
					php_eio_set_readdir_dent_and_names(args[1], req);
				} else { 
					/* If none of flags chosen. Not a good option, since in
					 * such case we've no info about offsets within EIO_BUF(req) ptr
					 * fill $result['names'] with dir names */
					php_eio_set_readdir_names(args[1], req);
				}

				break;

			default:
				ZVAL_LONG(args[1], EIO_RESULT(req));
		}
		/* }}} */

		if (!zend_is_callable(eio_cb->func, 0, &func_name TSRMLS_CC)) {
			EIO_REQ_WARN_INVALID_CB();
		} else {
			if (call_user_function(EG(function_table), NULL, eio_cb->func,
						&retval, 2, args TSRMLS_CC) == SUCCESS) {
				zval_dtor(&retval);
			}
		}

		efree(func_name);
		EIO_REQ_FREE_ARGS;
	}

	php_eio_free_eio_cb(eio_cb);

	return 0;
}
/*}}}*/

/* {{{ php_eio_want_poll_callback
 * Is called when eio wants attention(ready to process further requests) */
static void
php_eio_want_poll_callback(void) 
{
	int semid;

	semid = php_eio_bin_semaphore_get(PHP_EIO_SHM_KEY, 0);

	php_eio_bin_semaphore_post(semid);
}
/* }}} */

/* {{{ php_eio_done_poll_callback 
 * Is invoked when eio detects that all pending requests have been handled */
static void
php_eio_done_poll_callback(void) 
{
	int semid;

	semid = php_eio_bin_semaphore_get(PHP_EIO_SHM_KEY, 0);

	php_eio_bin_semaphore_wait(semid);
}
/* }}} */

#undef EIO_CB_ALLOC
#undef EIO_CB_SET_FIELD
#undef EIO_REQ_WARN_RESULT_ERROR
#undef EIO_REQ_WARN_INVALID_CB
#undef EIO_REQ_CB_INIT
#undef EIO_REQ_FREE_ARGS
#undef EIO_BUF_ZVAL_P
#undef EIO_BUF_FETCH_FROM_ZVAL
#undef EIO_CB_CUSTOM_LOCK
#undef EIO_CB_CUSTOM_UNLOCK

/* }}} */

/* {{{ PHP_INI
 */
/* 
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("eio.global_value",	   "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_eio_globals, eio_globals)
	STD_PHP_INI_ENTRY("eio.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_eio_globals, eio_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(eio)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

	le_eio_grp = zend_register_list_destructors_ex(
			NULL, NULL, PHP_EIO_GRP_DESCRIPTOR_NAME,
			module_number);
	le_eio_req = zend_register_list_destructors_ex(
			NULL, NULL, PHP_EIO_REQ_DESCRIPTOR_NAME,
			module_number);

	/* {{{ Constants */

	/* {{{ EIO_PRI_* */
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_PRI_MIN);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_PRI_DEFAULT);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_PRI_MAX);
	/* }}} */

	/* {{{ EIO_READDIR_* */
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_READDIR_DENTS);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_READDIR_DIRS_FIRST);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_READDIR_STAT_ORDER);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_READDIR_FOUND_UNKNOWN);
	/* }}} */

	/* {{{ EIO_DT_* */
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_UNKNOWN);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_FIFO);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_CHR);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_MPC);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_DIR);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_NAM);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_BLK);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_MPB);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_REG);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_NWK);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_CMP);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_LNK);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_SOCK);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_DOOR);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_WHT);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DT_MAX);
	/* }}} */

	/* {{{ EIO_O_* flags for open() */
	EIO_REGISTER_LONG_CONSTANT(EIO_O_RDONLY, O_RDONLY);
	EIO_REGISTER_LONG_CONSTANT(EIO_O_WRONLY, O_WRONLY);
	EIO_REGISTER_LONG_CONSTANT(EIO_O_RDWR, O_RDWR);
	EIO_REGISTER_LONG_CONSTANT(EIO_O_NONBLOCK, O_NONBLOCK);
	EIO_REGISTER_LONG_CONSTANT(EIO_O_APPEND, O_APPEND);
	EIO_REGISTER_LONG_CONSTANT(EIO_O_CREAT, O_CREAT);
	EIO_REGISTER_LONG_CONSTANT(EIO_O_TRUNC, O_TRUNC);
	EIO_REGISTER_LONG_CONSTANT(EIO_O_EXCL, O_EXCL);
	EIO_REGISTER_LONG_CONSTANT(EIO_O_FSYNC, O_FSYNC);
	/* }}} */

	/* {{{ EIO_S_I* mode for eio_open() */
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IRUSR, S_IRUSR);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IWUSR, S_IWUSR);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IXUSR, S_IXUSR);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IRGRP, S_IRGRP);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IWGRP, S_IWGRP);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IXGRP, S_IXGRP);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IROTH, S_IROTH);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IWOTH, S_IWOTH);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IXOTH, S_IXOTH);
	/* }}} */

	/* {{{ S_IF* used by eio_mknod() */
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IFREG, S_IFREG);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IFCHR, S_IFCHR);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IFBLK, S_IFBLK);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IFIFO, S_IFIFO);
	EIO_REGISTER_LONG_CONSTANT(EIO_S_IFSOCK, S_IFSOCK);
	/* }}} */

	/* {{{ EIO_SYNC_FILE_RANGE_* */
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_SYNC_FILE_RANGE_WAIT_BEFORE);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_SYNC_FILE_RANGE_WRITE);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_SYNC_FILE_RANGE_WAIT_AFTER);
	/* }}} */

	EIO_REGISTER_LONG_CONSTANT(EIO_FALLOC_FL_KEEP_SIZE, FALLOC_FL_KEEP_SIZE);

	/* }}} */

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(eio)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(eio)
{
	int semid;

	semid = php_eio_bin_semaphore_get(PHP_EIO_SHM_KEY, 0);
	if (semid != -1) {
		php_eio_bin_semaphore_dealloc(semid);
	}

	semid = php_eio_bin_semaphore_get(PHP_EIO_SHM_KEY, 
			PHP_EIO_SHM_PERM | IPC_CREAT | IPC_EXCL);
	if (semid == -1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, 
				"Failed initializing eio: %s", strerror(errno));
		return FAILURE;
	}

	if (eio_init(php_eio_want_poll_callback, 
				php_eio_done_poll_callback)) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, 
				"Failed initializing eio: %s", strerror(errno));
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(eio)
{
	int semid;

	if (eio_nreqs()) {
		EIO_EVENT_LOOP();
	}

	semid = php_eio_bin_semaphore_get(PHP_EIO_SHM_KEY, 0);
	if (semid != -1) {
		php_eio_bin_semaphore_dealloc(semid);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(eio)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "eio support", "enabled");
#ifdef EIO_DEBUG
	php_info_print_table_row(2, "Debug support", "enabled");
#else
	php_info_print_table_row(2, "Debug support", "disabled");
#endif
	php_info_print_table_row(2, "Version", PHP_EIO_VERSION);
	php_info_print_table_end();
}
/* }}} */


/* {{{ POSIX API wrappers */

/* {{{ proto bool eio_event_loop(void);
 * Polls eio until all requests proceeded. 
 * Returns TRUE on success, FALSE otherwise. */
PHP_FUNCTION(eio_event_loop)
{
	EIO_EVENT_LOOP();

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int eio_poll(void); 
 * Has to be called whenever there are pending requests that need finishing.
 * Applicable only when implementing userspace event loop. 
 * If any request invocation returns a non-zero value, returns that value.
 * Otherwise, it returns 0. */
PHP_FUNCTION(eio_poll) 
{
	RETURN_LONG(eio_poll());
}
/* }}} */

/* {{{ proto resource eio_open(string path, int flags, int mode, int pri, mixed callback = NULL[, mixed data = NULL]); 
 * Open a file. The file descriptor is passed as the second argument of the
 * event completion callback.
 *
 * flags	One of EIO_O_* constants, or their combinations.
 * EIO_O_* constants have the same meaning, as their corresponding O_*
 * counterparts defined in fnctl.h C header file. Default is EIO_O_RDWR.
 *
 * mode		One of EIO_S_I* constants or their combination. The constants have the
 * same meaning as their S_I* counterparts defined in sys/stat.h C header file.
 * Required, if a file is created. Ignored otherwise. 
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_open)
{
	char *path; 
	int path_len;
	long flags;
	long mode; 
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slllz!|z!",
				&path, &path_len,
				&flags, &mode,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	EIO_NEW_CB(eio_cb, callback, data);
	if (!mode) {
		mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	}
	if (!flags) {
		flags = O_RDWR /*| O_CREAT*/;
	}

	req = eio_open(path, flags, (mode_t) mode, 
			pri, php_eio_res_cb, eio_cb);

	EIO_RET_REQ_RESOURCE(req, eio_open);
}
/* }}} */

/* {{{ proto resource eio_truncate(string path[, int offset = 0[, int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]]); 
 * Truncate a file. 
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_truncate)
{
	char *path; 
	int path_len;
	long offset = 0;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|llz!z!",
				&path, &path_len, 
				&offset, &pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	if (offset < 0) {
		offset = 0;
	}

#ifdef EIO_DEBUG
	EIO_CHECK_WRITABLE(path, file);
#endif

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_truncate(path, offset, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_truncate);
}
/* }}} */

/* {{{ proto resource eio_chown(string path, int uid[, int gid = -1 [,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]]); 
 * Change file/direcrory permissions. uid is user ID. gid is group ID. uid/gid
 * is ignored when it's value is -1. Returns request resource on success, otherwise FALSE. 
 * The function always returns FALSE for NETWARE and WINDOWS */
PHP_FUNCTION(eio_chown)
{
#if defined(WINDOWS) || defined(NETWARE)
	errno = ENOSYS;
	RETURN_FALSE;
#else
	char *path; 
	int path_len;
	long uid;
	long gid = -1;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|llz!z!",
				&path, &path_len, 
				&uid, &gid, 
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);
#ifdef EIO_DEBUG
	EIO_CHECK_WRITABLE(path, file);
#endif

	if (uid < 0 && gid < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, 
				"invalid uid and/or gid");
		RETURN_FALSE;
	}

	if (access(path, W_OK) != 0) {
#ifdef EIO_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, 
				"path '%s' is not writable", path);
#endif
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_chown(path, (uid_t) uid, (gid_t) gid, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_chown);
#endif
}
/* }}} */

/* {{{ proto bool eio_chmod(string path, int mode, [int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Change file/direcrory permissions system call. Returns request resource on success, otherwise FALSE. 
 * The function always returns FALSE for NETWARE and WINDOWS */
PHP_FUNCTION(eio_chmod)
{
#if defined(WINDOWS) || defined(NETWARE)
	RETURN_FALSE;
#else
	char *path; 
	int path_len;
	long mode;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|lz!z!",
				&path, &path_len, 
				&mode, &pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

#ifdef EIO_DEBUG
	EIO_CHECK_WRITABLE(path, directory);
#endif

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_chmod(path, mode, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_chmod);
#endif
}
/* }}} */

/* {{{ proto resource eio_mkdir (string path, int mode, [int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Creates a directory. Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_mkdir)
{
	char *path; 
	int path_len;
	long mode;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|lz!z!",
				&path, &path_len, 
				&mode, &pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	if (access(path, F_OK) == 0) {
#ifdef EIO_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, 
				"directory '%s' already exists", path);
#endif
		RETURN_FALSE;	
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_mkdir(path, mode, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_mkdir);
}
/* }}} */

/* {{{ proto resource eio_rmdir (string path[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Removes a directory. Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_rmdir)
{
	char *path; 
	int path_len;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lz!z!",
				&path, &path_len, 
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	if (access(path, F_OK) != 0) {
#ifdef EIO_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, 
				"directory '%s' is not accessible", path);
#endif
		RETURN_FALSE;	
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_rmdir(path, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_rmdir);
}
/* }}} */

/* {{{ proto resource eio_unlink(string path[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Removes a file. Returns TRUE on success, otherwise FALSE. */
PHP_FUNCTION(eio_unlink)
{
	char *path; 
	int path_len;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lz!z!",
				&path, &path_len, 
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	if (access(path, F_OK) != 0) {
		/* Nothing to unlink */
		RETURN_TRUE;	
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_unlink(path, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_unlink);
}
/* }}} */

/* {{{ proto bool eio_utime(string path, double atime, double mtime[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Change file last access and modification times. 
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_utime)
{
	char *path; 
	int path_len;
	double atime, mtime;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sd/d/|lz!z!",
				&path, &path_len, 
				&atime, &mtime, 
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

#ifdef EIO_DEBUG
	EIO_CHECK_WRITABLE(path, file);
#endif

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_utime(path, (eio_tstamp) atime, (eio_tstamp) mtime, 
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_utime);
}
/* }}} */

/* {{{ proto resource eio_mknod(string path, int mode, int dev[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Create a special or ordinary file.
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_mknod)
{
	char *path; 
	int path_len;
	unsigned long mode, dev;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll|lz!z!",
				&path, &path_len, 
				&mode, &dev, 
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_mknod(path, (mode_t) mode, (dev_t) dev, 
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_mknod);
}
/* }}} */

/* {{{ proto resource eio_link(string path, string new_path[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Create a hardlink for file
 * Returns request resource on success, otherwise FALSE. Always returns FALSE for Windows and Netware. */
PHP_FUNCTION(eio_link)
{
#if defined(WINDOWS) || defined(NETWARE)
	errno = ENOSYS;
	RETURN_FALSE;
#else
	char *path, *new_path; 
	int path_len, new_path_len;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|lz!z!",
				&path, &path_len, 
				&new_path, &new_path_len, 
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);
	EIO_CHECK_PATH_LEN(new_path, new_path_len);

	/*EIO_CHECK_READABLE(path, file);*/

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_link(path, new_path,
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_link);
#endif
}
/* }}} */
	
/* {{{ proto resource eio_symlink(string path, string new_path[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Create a symlink for file
 * Returns resource on success, otherwise FALSE. Always returns FALSE for Windows and Netware. */
PHP_FUNCTION(eio_symlink)
{
#if defined(WINDOWS) || defined(NETWARE)
	errno = ENOSYS;
	RETURN_FALSE;
#else
	char *path, *new_path; 
	int path_len, new_path_len;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|lz!z!",
				&path, &path_len, 
				&new_path, &new_path_len, 
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);
	EIO_CHECK_PATH_LEN(new_path, new_path_len);

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_symlink(path, new_path,
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_symlink);
#endif
}
/* }}} */

/* {{{ proto resource eio_rename(string path, string new_path[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Change the name or location of a file.
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_rename)
{
	char *path, *new_path; 
	int path_len, new_path_len;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|lz!z!",
				&path, &path_len, 
				&new_path, &new_path_len, 
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);
	EIO_CHECK_PATH_LEN(new_path, new_path_len);

#ifdef EIO_DEBUG
	EIO_CHECK_WRITABLE(path, file);
#endif

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_rename(path, new_path,
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_rename);
}
/* }}} */

/* {{{ proto resource eio_close(int fd[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Closes file. Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_close)
{
	unsigned long fd;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/|lz!z!",
				&fd,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}
	if (!fd) {
#ifdef EIO_DEBUG
	php_error_docref(NULL TSRMLS_CC, E_ERROR,
			"invalid file descriptor '%ld'", fd);
#endif
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_close(fd, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_close);
}
/* }}} */

/* {{{ proto resource eio_sync([int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Commit buffer cache to disk. 
 * Returns request resource on success, otherwise FALSE. Always returns FALSE for Windows and Netware. */
PHP_FUNCTION(eio_sync)
{
#if defined(WINDOWS) || defined(NETWARE)
	errno = ENOSYS;
	RETURN_FALSE;
#else
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lz!z!",
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_sync(pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_sync);
#endif
}
/* }}} */

/* {{{ proto resource feio_fsync(int fd[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Synchronize a file's in-core state with storage device. 
 * Returns request resource on success, otherwise FALSE. Always returns FALSE for Windows and Netware. */
PHP_FUNCTION(eio_fsync)
{
#if defined(WINDOWS) || defined(NETWARE)
	errno = ENOSYS;
	RETURN_FALSE;
#else
	unsigned long fd;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|lz!z!",
				&fd,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_fsync(fd, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_fsync);
#endif
}
/* }}} */

/* {{{ proto resource eio_datafsync(int fd[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Synchronize a file's in-core state with storage device. 
 * Returns request resource on success, otherwise FALSE. Always returns FALSE for Windows and Netware. */
PHP_FUNCTION(eio_fdatasync)
{
#if defined(WINDOWS) || defined(NETWARE)
	errno = ENOSYS;
	RETURN_FALSE;
#else
	unsigned long fd;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|lz!z!",
				&fd,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_fdatasync(fd, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_fdatasync);
#endif
}
/* }}} */

/* {{{ proto resource eio_futime(int fd, double atime, double mtime[ ,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Change file last access and modification times by file descriptor fd.
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_futime)
{
	unsigned long fd;
	double atime, mtime;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/l/l/|lz!z!",
				&fd,
				&atime, &mtime, 
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_futime(fd, (eio_tstamp) atime, (eio_tstamp) mtime, 
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_futime);
}
/* }}} */

/* {{{ proto resource eio_ftruncate(int fd[, int offset = 0[, int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]]); 
 * Truncate a file. Returns resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_ftruncate)
{
	unsigned long fd, offset = 0;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|llz!z!",
				&fd, &offset, &pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	if (offset < 0) {
		offset = 0;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_ftruncate(fd, offset, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_ftruncate);
}
/* }}} */

/* {{{ proto resource eio_fchmod(int fd, int mode, [int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Change file permissions system call by file descriptor fd.
 * Returns resource on success, otherwise FALSE. 
 * The function always returns FALSE for NETWARE and WINDOWS */
PHP_FUNCTION(eio_fchmod)
{
#if defined(WINDOWS) || defined(NETWARE)
	RETURN_FALSE;
#else
	unsigned long fd, mode;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/l/|lz!z!",
				&fd, &mode, &pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_fchmod(fd, mode, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_fchmod);
#endif
}
/* }}} */

/* {{{ proto resource eio_fchown(int fd, int uid[, int gid = -1 [,int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]]); 
 * Change file/direcrory permissions by file descriptor. uid is user ID. gid is group ID. uid/gid
 * is ignored when it's value is -1. 
 * Returns resource on success, otherwise FALSE. 
 * The function always returns FALSE for NETWARE and WINDOWS */
PHP_FUNCTION(eio_fchown)
{
#if defined(WINDOWS) || defined(NETWARE)
	RETURN_FALSE;
#else
	unsigned long fd, uid = -1, gid = -1;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/l/|l/lz!z!",
				&fd, &uid, &gid, 
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	if (uid < 0 || gid < 0) {
#ifdef EIO_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_WARNING, 
				"invalid uid and/or gid");
#endif
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_fchown(fd, (uid_t) uid, (gid_t) gid, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_fchown);
#endif
}
/* }}} */

/* {{{ proto bool eio_dup2(int fd, int fd2, [int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]); 
 * Duplicate a file descriptor.
 * Returns TRUE on success, otherwise FALSE. */
PHP_FUNCTION(eio_dup2)
{
	unsigned long fd, fd2;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/l/|lz!z!",
				&fd, &fd2, &pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_dup2(fd, fd2, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_dup2);
}
/* }}} */

/* {{{ proto resource eio_read(int fd, int length, int offset, int pri, mixed callback[, mixed data = NULL]); 
 * Read from a file descriptor, fd, at a given offset.
 *
 * length specifies amount of bytes to read. 
 *
 * offset is offset from the beginning of the file. Default: 0.
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_read)
{
	unsigned long fd, length = 0, offset = 0;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/lllz!|z!",
				&fd, &length, &offset,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);
	if (!fd) {
#ifdef EIO_DEBUG
	php_error_docref(NULL TSRMLS_CC, E_ERROR,
			"invalid file descriptor '%ld'", fd);
#endif
		RETURN_FALSE;
	}

	/* Actually, second parameter is buffer for read contents. 
	 * But eio allocates memory for it's eio_req->ptr2 internally, 
	 * and passes it to the callback. The buffer with read contents will be
	 * available in callback as EIO_BUF(req). Thus, we don't need allocate
	 * memory ourselves here */
	req = eio_read(fd, 0, length, offset,
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_read);
}
/* }}} */

/* {{{ proto resource eio_write(int fd, mixed &str[, int length = NULL[, int offset = 0[, int pri = 0 [, mixed callback = NULL[, mixed data = NULL]]]]]); 
 * Writes string to the file specified by file descriptor fd.
 *
 * length	amount of characters to write. If is NULL, entire string is
 * written to the file
 *
 * offset	offset from the beginning of the buf contents. Default: 0.
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_write)
{
	zval *zbuf;
	int num_bytes;
	unsigned long fd, length = 0, offset = 0;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/z|lllz!z!",
				&fd, &zbuf, 
				&length, &offset,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	if (!fd) {
#ifdef EIO_DEBUG
	php_error_docref(NULL TSRMLS_CC, E_ERROR,
			"invalid file descriptor '%ld'", fd);
#endif
		RETURN_FALSE;
	}

	convert_to_string(zbuf);

	if (Z_STRLEN_P(zbuf) < (length + offset)) {
		RETURN_FALSE;
	}

	if (ZEND_NUM_ARGS() == 2 || length <= 0) {
		num_bytes = Z_STRLEN_P(zbuf);
	} else {
		num_bytes = length;	
	}	

	if (!num_bytes) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_write(fd, Z_STRVAL_P(zbuf), num_bytes, offset,
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_write);
}
/* }}} */

/* {{{ proto resource eio_readlink(string path, int pri, mixed callback[, mixed data = NULL]); 
 * Read value of a symbolic link.
 *
 * Returns TRUE on success, otherwise FALSE. 
 * Always returns FALSE for Netware and Windows. */
PHP_FUNCTION(eio_readlink)
{
#if defined(WINDOWS) || defined(NETWARE)
	errno = ENOSYS;
	RETURN_FALSE;
#else
	char *path; 
	int path_len;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slz!|z!",
				&path, &path_len,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	EIO_CHECK_READABLE(path, file);

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_readlink(path, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_readlink);
#endif
}
/* }}} */

/* {{{ proto resource eio_realpath(string path, int pri, mixed callback[, mixed data = NULL]); 
 * Return the canonicalized absolute pathname in the second argument of the
 * event completion callback.
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_realpath)
{
	char *path; 
	int path_len;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slz!|z!",
				&path, &path_len,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_realpath(path, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_realpath);
}
/* }}} */

/* {{{ proto resource eio_stat(string path, int pri, mixed callback[, mixed data = NULL]); 
 * Get file status
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_stat)
{
	char *path; 
	int path_len;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slz!|z!",
				&path, &path_len,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_stat(path, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_stat);
}
/* }}} */

/* {{{ proto resource eio_lstat(string path, int pri, mixed callback[, mixed data = NULL]); 
 * Get file status
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_lstat)
{
	char *path; 
	int path_len;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slz!|z!",
				&path, &path_len,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_lstat(path, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_lstat);
}
/* }}} */

/* {{{ proto resource eio_fstat(int fd, int pri, mixed callback[, mixed data = NULL]); 
 * Get file status
 *
 * fd	file descriptor
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_fstat)
{
	unsigned long fd;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/lz!|z!",
				&fd,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_fstat(fd, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_fstat);
}
/* }}} */

/* {{{ proto resource eio_statvfs(string path, int pri, mixed callback[, mixed data = NULL]); 
 * Get file system statistics
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_statvfs)
{
	char *path; 
	int path_len;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slz!|z!",
				&path, &path_len,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_statvfs(path, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_statvfs);
}
/* }}} */

/* {{{ proto resource eio_fstatvfs(int fd, int pri, mixed callback[, mixed data = NULL]); 
 * Get file system statistics
 *
 * fd	file descriptor
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_fstatvfs)
{
	unsigned long fd;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/lz!|z!",
				&fd,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_fstatvfs(fd, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_fstatvfs);
}
/* }}} */

/* }}} */


/* {{{ proto resource eio_readdir(string path, int flags, int pri, mixed callback[, mixed data = NULL]); 
 * Reads through a whole directory(via the opendir, readdir and closedir system
 * calls) and returns either the names or an array,
 * depending on the flags argument.
 *
 * flags	combination of EIO_READDIR_* constants
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_readdir)
{
	char *path; 
	int path_len;
	int flags;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sllz|z!",
				&path, &path_len, &flags,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	/* In current version of eio it causes SEGVAULT without the following */
	if (flags & (EIO_READDIR_DIRS_FIRST | EIO_READDIR_STAT_ORDER)) {
		flags |= EIO_READDIR_DENTS;
	}

	req = eio_readdir(path, flags, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_readdir);
}
/* }}} */


/* {{{ OS-SPECIFIC CALL WRAPPERS */
#ifndef EIO_NON_LINUX

/* {{{ proto resource eio_sendfile(int out_fd, int int in_fd, int offset, int length[, int pri[, mixed callback[, mixed data = NULL]]]); 
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_sendfile)
{
	long out_fd, in_fd, offset, length;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/l/ll|lz!z!",
				&out_fd, &in_fd, &offset, &length,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_sendfile(out_fd, in_fd, offset, length, 
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_sendfile);
}
/* }}} */

/* {{{ proto resource eio_readahead(int fd, int offset, int length[, int pri[, mixed callback[, mixed data = NULL]]]); 
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_readahead)
{
	long fd, offset, length;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/ll|lz!z!",
				&fd, &offset, &length,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_readahead(fd, offset, length, 
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_readahead);
}
/* }}} */

/* {{{ proto resource eio_syncfs(int fd[, int pri[, mixed callback[, mixed data = NULL]]]); 
 *
 * Returns resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_syncfs)
{
	long fd;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/|lz!z!",
				&fd, &pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_syncfs(fd, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_syncfs);
}
/* }}} */

/* {{{ proto resource eio_sync_file_range(int fd, int offset, int nbytes, int flags[, int pri[, mixed callback[, mixed data = NULL]]]); 
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_sync_file_range)
{
	unsigned long fd, offset, nbytes, flags;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/lll|lz!z!",
				&fd, &offset, &nbytes, &flags,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_sync_file_range(fd, offset, nbytes, flags, 
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_sync_file_range);
}
/* }}} */

/* {{{ proto resource eio_fallocate(int fd, int mode, int offset, int length[, int pri[, mixed callback[, mixed data = NULL]]]); 
 * Allows the caller to directly manipulate the allocated disk space for the
 * file referred to by fd for the byte range starting at offset and continuing
 * for length bytes.
 *
 * Currently only one flag is supported for mode: EIO_FALLOC_FL_KEEP_SIZE (the
 * same as system constant FALLOC_FL_KEEP_SIZE).
 *
 * NOTE: the file should be opened for writing. EIO_O_CREAT should be logically
 * OR'd with EIO_O_WRONLY or EIO_O_RDWR
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_fallocate)
{
	unsigned long fd, mode = 0, offset = 0, length;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l/lll|lz!z!",
				&fd, &mode, &offset, &length,
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_fallocate(fd, mode, offset, length, 
			pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_fallocate);
}
/* }}} */

#endif

/* }}} */


/* {{{ EIO-SPECIFIC REQUESTS */

/* {{{ proto resource eio_custom(mixed execute, int pri, mixed callback[, mixed data = NULL]); 
 * Execute custom user function specified by execute argument just like other eio_* calls. 
 *
 * execute	User function that should match the following prototype:
 * mixed execute(mixed data);
 *
 * callback	The same event completion callback that should match the following
 * prototype:
 * void callback(mixed data, mixed result);
 * data		data passed to execute via data argument without modifications
 * result	value returned by execute
 *
 * data		Optional argument passed to execute
 *
 * Returns request resource on success, otherwise FALSE. 
 * TODO: WARNING! Craches with xdebug extension. See issue http://bugs.xdebug.org/view.php?id=725
 */
PHP_FUNCTION(eio_custom)
{
	EIO_INIT_CUSTOM(pri, callback, data, execute, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zlz|z!",
				&execute, &pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}
	
	eio_cb = php_eio_new_eio_cb_custom(callback, data, execute TSRMLS_CC);

	req = eio_custom(php_eio_custom_execute,
			pri, php_eio_res_cb_custom, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_custom);

}
/* }}} */

/* {{{ proto resource eio_busy(int delay[, int pri=EIO_PRI_DEFAULT[, mixed callback=NULL[, mixed data=NULL]]]); 
 * Artificially increase load. Could be useful in tests, benchmarking. 
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_busy)
{
	unsigned long delay;
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|lz!z!",
				&delay, &pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_busy(delay, pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_busy);
}
/* }}} */

/* {{{ proto resource eio_nop([int pri[, mixed callback[, mixed data = NULL]]]); 
 * Does nothing, except go through the whole request cycle.
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_nop)
{
	EIO_INIT(pri, callback, data, eio_cb, req);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lz!z!",
				&pri, &callback, &data) == FAILURE) {
		RETURN_FALSE;
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_nop(pri, php_eio_res_cb, eio_cb);
	EIO_RET_REQ_RESOURCE(req, eio_nop);
}
/* }}} */

/* }}} */

/* {{{ proto void eio_cancel(resource req); 
 * Cancels a request */
PHP_FUNCTION(eio_cancel)
{
	zval *zreq; 
	eio_req *req; 

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zreq) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(req, eio_req*, &zreq, -1,
			PHP_EIO_REQ_DESCRIPTOR_NAME, le_eio_req);

	if (req->type != EIO_CUSTOM) {
		eio_cancel(req);
		php_eio_free_eio_cb(req->data);
	} else if (!EIO_CB_CUSTOM_IS_LOCKED((php_eio_cb_custom_t *)req->data)) {
		eio_cancel(req);
		php_eio_free_eio_cb_custom(req->data);
	}
}
/* }}} */

#undef EIO_CB_CUSTOM_IS_LOCKED

/* {{{ GROUPING AND LIMITING REQUESTS */

/* {{{ proto resource eio_grp(mixed callback[, mixed data = NULL]); 
 * Creates, submits and returns a group request resource.
 *
 * Returns group request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_grp)
{
	zval *callback, *data = NULL;
	php_eio_cb_t *eio_cb;
	eio_req *req;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!",
				&callback, &data) == FAILURE) {
		RETURN_NULL();
	}

	EIO_NEW_CB(eio_cb, callback, data);

	req = eio_grp(php_eio_res_cb, eio_cb);
	EIO_RET_IF_FAILED(req, eio_grp);

	ZEND_REGISTER_RESOURCE(return_value, req,
			le_eio_grp);
}
/* }}} */

/* {{{ proto void eio_grp_add(resource grp, resource req); 
 * Adds a request to the request group. */
PHP_FUNCTION(eio_grp_add)
{
	zval *zgrp, *zreq;
	eio_req *grp, *req;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr/",
				&zgrp, &zreq) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(grp, eio_req*, &zgrp, -1,
			PHP_EIO_GRP_DESCRIPTOR_NAME, le_eio_grp);
	ZEND_FETCH_RESOURCE(req, eio_req*, &zreq, -1,
			PHP_EIO_REQ_DESCRIPTOR_NAME, le_eio_req);

	grp->result = 0;
	eio_grp_add(grp, req);
}
/* }}} */

/* {{{ proto void eio_grp_limit(resource grp, int limit); 
 * Set group limit */
PHP_FUNCTION(eio_grp_limit)
{
	zval *zgrp;
	eio_req *grp;
	long limit;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r/l",
				&zgrp, &limit) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(grp, eio_req*, &zgrp, -1,
			PHP_EIO_GRP_DESCRIPTOR_NAME, le_eio_grp);

	eio_grp_limit(grp, limit);
}
/* }}} */

/* {{{ proto void eio_grp_cancel(resource grp); 
 * Cancels request group */
PHP_FUNCTION(eio_grp_cancel)
{
	zval *zgrp;
	eio_req *grp;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
				&zgrp) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(grp, eio_req*, &zgrp, -1,
			PHP_EIO_GRP_DESCRIPTOR_NAME, le_eio_grp);

	grp->result = -1;
	eio_grp_cancel(grp);
}
/* }}} */

/* }}} */

/* {{{ CONFIGURATION */

/* {{{ proto void eio_set_max_poll_time(float nseconds); 
 * Limits the amount of time spent handling eio requests in eio_poll() */
PHP_FUNCTION(eio_set_max_poll_time)
{
	double nseconds;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d",
				&nseconds) == FAILURE) {
		return;
	}

	eio_set_max_poll_time((eio_tstamp)nseconds);
}
/* }}} */

/* Just another quickies for simple functions */
#define EIO_SET_INT_FUNCTION(eio_func)							\
PHP_FUNCTION(eio_func)											\
{																\
	unsigned int num;											\
																\
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",	\
				&num) == FAILURE) {								\
		return;													\
	}															\
	eio_func(num);												\
}

#define EIO_GET_INT_FUNCTION(eio_func)							\
PHP_FUNCTION(eio_func)											\
{																\
	RETURN_LONG(eio_func());									\
}

EIO_SET_INT_FUNCTION(eio_set_max_poll_reqs);
EIO_SET_INT_FUNCTION(eio_set_min_parallel);
EIO_SET_INT_FUNCTION(eio_set_max_parallel);
EIO_SET_INT_FUNCTION(eio_set_max_idle);

EIO_GET_INT_FUNCTION(eio_nthreads);
EIO_GET_INT_FUNCTION(eio_nreqs);
EIO_GET_INT_FUNCTION(eio_nready);
EIO_GET_INT_FUNCTION(eio_npending);

#undef EIO_SET_INT_FUNCTION
#undef EIO_GET_INT_FUNCTION
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
