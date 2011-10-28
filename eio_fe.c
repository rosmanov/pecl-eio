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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#include "eio_fe.h"

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

EIO_ARGINFO_FUNC_0(get_eventfd);

#undef EIO_ARGINFO
#undef EIO_ARGINFO_FUNC_0
#undef EIO_ARGINFO_FUNC_1
#undef EIO_ARGINFO_PRI_CB_DATA
#undef EIO_ARGINFO_FUNC_1_N
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

	PHP_FE(eio_sendfile, arginfo_eio_sendfile)
	PHP_FE(eio_readahead, arginfo_eio_readahead)
	PHP_FE(eio_syncfs, arginfo_eio_syncfs)
	PHP_FE(eio_sync_file_range, arginfo_eio_sync_file_range)
	PHP_FE(eio_fallocate, arginfo_eio_fallocate)

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
	PHP_FE(eio_get_eventfd, arginfo_eio_get_eventfd)
	{NULL, NULL, NULL}	/* Must be the last line in eio_functions[] */
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
