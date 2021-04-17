<?php
/** @generate-function-entries */

function eio_event_loop(): bool {}
function eio_poll(): int {}
/**
 * @return resource|false
 */
function eio_open(string $path, int $flags, int $mode, int $pri, ?mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|false
 */
function eio_truncate(string $path, int $offset = 0, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|false
 */
function eio_mkdir(string $path, int $mode, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_rmdir(string $path, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_unlink(string $path, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_utime(string $path, float $atime, float $mtime, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_mknod(string $path, int $mode, int $dev, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_link(string $path, string $new_path, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_symlink(string $path, string $new_path, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_rename(string $path, string $new_path, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_close(mixed $fd, int $pri = 0, ?mixed $callback = null, ?mixed $data = null){}
/**
 * @return resource|bool
 */
function eio_sync(int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_fsync(mixed $fd, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_fdatasync(mixed $fd, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_futime(mixed $fd, float $atime, float $mtime, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_ftruncate(mixed $fd, int $offset = 0, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_chmod(string $path, int $mode, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_fchmod(mixed $fd, int $mode, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_chown(string $path, int $uid, int $gid = -1, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_fchown(mixed $fd, int $uid, int $gid = -1, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_dup2(mixed $fd, mixed $fd2, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_read(mixed $fd, int $length, int $offset, int $pri, mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_write(mixed $fd, mixed $str, int $length = 0, int $offset = 0, int $pri = 0, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_readlink(string $path, int $pri, mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_realpath(string $path, int $pri, mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_stat(string $path, int $pri, mixed $callback, ?mixed $data = NULL) {}
/**
 * @return resource|bool
 */
function eio_lstat(string $path, int $pri, mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_fstat(mixed $fd, int $pri, mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_statvfs(string $path, int $pri, mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_fstatvfs(mixed $fd, int $pri, mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_readdir(string $path, int $flags, int $pri, mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_sendfile(mixed $out_fd, mixed $in_fd, int $offset, int $length, int $pri, mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_readahead(mixed $fd, int $offset, int $length, int $pri = EIO_PRI_DEFAULT, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_seek(mixed $fd, int $offset, int $whence, int $pri = EIO_PRI_DEFAULT, ?mixed $callback = null, ?mixed $data = NULL) {}
/**
 * @return resource|bool
 */
function eio_syncfs(mixed $fd, int $pri = EIO_PRI_DEFAULT, ?mixed $callback = null, mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_sync_file_range(mixed $fd, int $offset, int $nbytes, int $flags, int $pri = EIO_PRI_DEFAULT, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_fallocate(mixed $fd, int $mode, int $offset, int $length, int $pri = EIO_PRI_DEFAULT, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_custom(mixed $execute, int $pri, mixed $callback, ?mixed $data = null) {}
/**
 * @return resource|bool
 */
function eio_busy(int $delay, int $pri=EIO_PRI_DEFAULT, ?mixed $callback=null, ?mixed $data=null) {}
/**
 * @return resource|bool
 */
function eio_nop(int $pri = EIO_PRI_DEFAULT, ?mixed $callback = null, ?mixed $data = null) {}
/**
 * @param resource $req
 */
function eio_cancel($req): void {}

/**
 * @return resource
 */
function eio_grp(mixed $callback, ?mixed $data = null) {}
/**
 * @param resource $grp
 * @param resource $req
 */
function eio_grp_add($grp, $req): void {}
/**
 * @param resource $grp
 * @param int $limit
 */
function eio_grp_limit($grp, int $limit): void {}
/**
 * @param resource $grp
 */
function eio_grp_cancel($grp): void {}

function eio_set_max_poll_time(float $nseconds): void {}

function eio_set_max_poll_reqs(int $value): void {}
function eio_set_min_parallel(int $value): void {}
function eio_set_max_parallel(int $value): void {}
function eio_set_max_idle(int $value): void {}

function eio_nthreads(): int {}
function eio_nreqs(): int {}
function eio_nready(): int {}
function eio_npending(): int {}

/**
 * @return resource|null
 */
function eio_get_event_stream() {}
/**
 * @param resource $req
 */
function eio_get_last_error($req): string {}
