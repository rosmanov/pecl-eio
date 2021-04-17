/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 7b2cd98c28b75f56e2000c33974a3340dc391ed0 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_eio_event_loop, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_eio_poll, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_open, 0, 0, 5)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, pri, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 1)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_truncate, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_mkdir, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_rmdir, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_eio_unlink arginfo_eio_rmdir

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_utime, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, atime, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, mtime, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_mknod, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, dev, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_link, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, new_path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_eio_symlink arginfo_eio_link

#define arginfo_eio_rename arginfo_eio_link

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_close, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_sync, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_eio_fsync arginfo_eio_close

#define arginfo_eio_fdatasync arginfo_eio_close

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_futime, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, atime, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, mtime, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_ftruncate, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_eio_chmod arginfo_eio_mkdir

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_fchmod, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_chown, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, uid, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, gid, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_fchown, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, uid, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, gid, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_dup2, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, fd2, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_read, 0, 0, 5)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, pri, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_write, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, str, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_readlink, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, pri, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_eio_realpath arginfo_eio_readlink

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_stat, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, pri, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "NULL")
ZEND_END_ARG_INFO()

#define arginfo_eio_lstat arginfo_eio_readlink

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_fstat, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, pri, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_eio_statvfs arginfo_eio_readlink

#define arginfo_eio_fstatvfs arginfo_eio_fstat

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_readdir, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, pri, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_sendfile, 0, 0, 6)
	ZEND_ARG_TYPE_INFO(0, out_fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, in_fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, pri, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_readahead, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "EIO_PRI_DEFAULT")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_seek, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, whence, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "EIO_PRI_DEFAULT")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "NULL")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_syncfs, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "EIO_PRI_DEFAULT")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 0, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_sync_file_range, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, nbytes, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "EIO_PRI_DEFAULT")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_fallocate, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "EIO_PRI_DEFAULT")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_custom, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, execute, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, pri, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_busy, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, delay, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "EIO_PRI_DEFAULT")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_nop, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pri, IS_LONG, 0, "EIO_PRI_DEFAULT")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_MIXED, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_eio_cancel, 0, 1, IS_VOID, 0)
	ZEND_ARG_INFO(0, req)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_grp, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, data, IS_MIXED, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_eio_grp_add, 0, 2, IS_VOID, 0)
	ZEND_ARG_INFO(0, grp)
	ZEND_ARG_INFO(0, req)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_eio_grp_limit, 0, 2, IS_VOID, 0)
	ZEND_ARG_INFO(0, grp)
	ZEND_ARG_TYPE_INFO(0, limit, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_eio_grp_cancel, 0, 1, IS_VOID, 0)
	ZEND_ARG_INFO(0, grp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_eio_set_max_poll_time, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, nseconds, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_eio_set_max_poll_reqs, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_eio_set_min_parallel arginfo_eio_set_max_poll_reqs

#define arginfo_eio_set_max_parallel arginfo_eio_set_max_poll_reqs

#define arginfo_eio_set_max_idle arginfo_eio_set_max_poll_reqs

#define arginfo_eio_nthreads arginfo_eio_poll

#define arginfo_eio_nreqs arginfo_eio_poll

#define arginfo_eio_nready arginfo_eio_poll

#define arginfo_eio_npending arginfo_eio_poll

ZEND_BEGIN_ARG_INFO_EX(arginfo_eio_get_event_stream, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_eio_get_last_error, 0, 1, IS_STRING, 0)
	ZEND_ARG_INFO(0, req)
ZEND_END_ARG_INFO()


ZEND_FUNCTION(eio_event_loop);
ZEND_FUNCTION(eio_poll);
ZEND_FUNCTION(eio_open);
ZEND_FUNCTION(eio_truncate);
ZEND_FUNCTION(eio_mkdir);
ZEND_FUNCTION(eio_rmdir);
ZEND_FUNCTION(eio_unlink);
ZEND_FUNCTION(eio_utime);
ZEND_FUNCTION(eio_mknod);
ZEND_FUNCTION(eio_link);
ZEND_FUNCTION(eio_symlink);
ZEND_FUNCTION(eio_rename);
ZEND_FUNCTION(eio_close);
ZEND_FUNCTION(eio_sync);
ZEND_FUNCTION(eio_fsync);
ZEND_FUNCTION(eio_fdatasync);
ZEND_FUNCTION(eio_futime);
ZEND_FUNCTION(eio_ftruncate);
ZEND_FUNCTION(eio_chmod);
ZEND_FUNCTION(eio_fchmod);
ZEND_FUNCTION(eio_chown);
ZEND_FUNCTION(eio_fchown);
ZEND_FUNCTION(eio_dup2);
ZEND_FUNCTION(eio_read);
ZEND_FUNCTION(eio_write);
ZEND_FUNCTION(eio_readlink);
ZEND_FUNCTION(eio_realpath);
ZEND_FUNCTION(eio_stat);
ZEND_FUNCTION(eio_lstat);
ZEND_FUNCTION(eio_fstat);
ZEND_FUNCTION(eio_statvfs);
ZEND_FUNCTION(eio_fstatvfs);
ZEND_FUNCTION(eio_readdir);
ZEND_FUNCTION(eio_sendfile);
ZEND_FUNCTION(eio_readahead);
ZEND_FUNCTION(eio_seek);
ZEND_FUNCTION(eio_syncfs);
ZEND_FUNCTION(eio_sync_file_range);
ZEND_FUNCTION(eio_fallocate);
ZEND_FUNCTION(eio_custom);
ZEND_FUNCTION(eio_busy);
ZEND_FUNCTION(eio_nop);
ZEND_FUNCTION(eio_cancel);
ZEND_FUNCTION(eio_grp);
ZEND_FUNCTION(eio_grp_add);
ZEND_FUNCTION(eio_grp_limit);
ZEND_FUNCTION(eio_grp_cancel);
ZEND_FUNCTION(eio_set_max_poll_time);
ZEND_FUNCTION(eio_set_max_poll_reqs);
ZEND_FUNCTION(eio_set_min_parallel);
ZEND_FUNCTION(eio_set_max_parallel);
ZEND_FUNCTION(eio_set_max_idle);
ZEND_FUNCTION(eio_nthreads);
ZEND_FUNCTION(eio_nreqs);
ZEND_FUNCTION(eio_nready);
ZEND_FUNCTION(eio_npending);
ZEND_FUNCTION(eio_get_event_stream);
ZEND_FUNCTION(eio_get_last_error);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(eio_event_loop, arginfo_eio_event_loop)
	ZEND_FE(eio_poll, arginfo_eio_poll)
	ZEND_FE(eio_open, arginfo_eio_open)
	ZEND_FE(eio_truncate, arginfo_eio_truncate)
	ZEND_FE(eio_mkdir, arginfo_eio_mkdir)
	ZEND_FE(eio_rmdir, arginfo_eio_rmdir)
	ZEND_FE(eio_unlink, arginfo_eio_unlink)
	ZEND_FE(eio_utime, arginfo_eio_utime)
	ZEND_FE(eio_mknod, arginfo_eio_mknod)
	ZEND_FE(eio_link, arginfo_eio_link)
	ZEND_FE(eio_symlink, arginfo_eio_symlink)
	ZEND_FE(eio_rename, arginfo_eio_rename)
	ZEND_FE(eio_close, arginfo_eio_close)
	ZEND_FE(eio_sync, arginfo_eio_sync)
	ZEND_FE(eio_fsync, arginfo_eio_fsync)
	ZEND_FE(eio_fdatasync, arginfo_eio_fdatasync)
	ZEND_FE(eio_futime, arginfo_eio_futime)
	ZEND_FE(eio_ftruncate, arginfo_eio_ftruncate)
	ZEND_FE(eio_chmod, arginfo_eio_chmod)
	ZEND_FE(eio_fchmod, arginfo_eio_fchmod)
	ZEND_FE(eio_chown, arginfo_eio_chown)
	ZEND_FE(eio_fchown, arginfo_eio_fchown)
	ZEND_FE(eio_dup2, arginfo_eio_dup2)
	ZEND_FE(eio_read, arginfo_eio_read)
	ZEND_FE(eio_write, arginfo_eio_write)
	ZEND_FE(eio_readlink, arginfo_eio_readlink)
	ZEND_FE(eio_realpath, arginfo_eio_realpath)
	ZEND_FE(eio_stat, arginfo_eio_stat)
	ZEND_FE(eio_lstat, arginfo_eio_lstat)
	ZEND_FE(eio_fstat, arginfo_eio_fstat)
	ZEND_FE(eio_statvfs, arginfo_eio_statvfs)
	ZEND_FE(eio_fstatvfs, arginfo_eio_fstatvfs)
	ZEND_FE(eio_readdir, arginfo_eio_readdir)
	ZEND_FE(eio_sendfile, arginfo_eio_sendfile)
	ZEND_FE(eio_readahead, arginfo_eio_readahead)
	ZEND_FE(eio_seek, arginfo_eio_seek)
	ZEND_FE(eio_syncfs, arginfo_eio_syncfs)
	ZEND_FE(eio_sync_file_range, arginfo_eio_sync_file_range)
	ZEND_FE(eio_fallocate, arginfo_eio_fallocate)
	ZEND_FE(eio_custom, arginfo_eio_custom)
	ZEND_FE(eio_busy, arginfo_eio_busy)
	ZEND_FE(eio_nop, arginfo_eio_nop)
	ZEND_FE(eio_cancel, arginfo_eio_cancel)
	ZEND_FE(eio_grp, arginfo_eio_grp)
	ZEND_FE(eio_grp_add, arginfo_eio_grp_add)
	ZEND_FE(eio_grp_limit, arginfo_eio_grp_limit)
	ZEND_FE(eio_grp_cancel, arginfo_eio_grp_cancel)
	ZEND_FE(eio_set_max_poll_time, arginfo_eio_set_max_poll_time)
	ZEND_FE(eio_set_max_poll_reqs, arginfo_eio_set_max_poll_reqs)
	ZEND_FE(eio_set_min_parallel, arginfo_eio_set_min_parallel)
	ZEND_FE(eio_set_max_parallel, arginfo_eio_set_max_parallel)
	ZEND_FE(eio_set_max_idle, arginfo_eio_set_max_idle)
	ZEND_FE(eio_nthreads, arginfo_eio_nthreads)
	ZEND_FE(eio_nreqs, arginfo_eio_nreqs)
	ZEND_FE(eio_nready, arginfo_eio_nready)
	ZEND_FE(eio_npending, arginfo_eio_npending)
	ZEND_FE(eio_get_event_stream, arginfo_eio_get_event_stream)
	ZEND_FE(eio_get_last_error, arginfo_eio_get_last_error)
	ZEND_FE_END
};
