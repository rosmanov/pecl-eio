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
#ifndef EIO_FE_H
#  define EIO_FE_H

PHP_FUNCTION(eio_init);
PHP_FUNCTION(eio_poll);
PHP_FUNCTION(eio_event_loop);
PHP_FUNCTION(eio_get_last_error);

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

PHP_FUNCTION(eio_sendfile);
PHP_FUNCTION(eio_readahead);
PHP_FUNCTION(eio_seek);
PHP_FUNCTION(eio_syncfs);
PHP_FUNCTION(eio_sync_file_range);
PHP_FUNCTION(eio_fallocate);

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

PHP_FUNCTION(eio_get_event_stream);

#endif /* EIO_FE_H */
/* 
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
