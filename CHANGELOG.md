# Changelog

## 4.0.0 (2026-11-05)

- Dropped support for PHP versions older than 8.2.
- Fixed use-after-free of `eio_req` in PHP resources. Using a request or group resource after libeio destroyed the request is now a no-op, and `eio_get_last_error()` returns `false`.
- Fixed the macOS (Homebrew) build: libeio is now compiled as a separate unit.
- Fixed a double free in `eio_cancel()` when it is called twice on the same request.
- Detect PHP sockets headers at configure time.
- Added `composer.json` for [PIE](https://github.com/php/pie) support.
- Fixed PHP 8.6-rc build: replace the deleted `zval_dtor` with `zval_ptr_dtor_nogc`.

## 3.1.4

- Fixed #25: use `zend_ce_exception` instead of `zend_exception_get_default()` for PHP 8.5 (by Remi Collet).

## 3.1.3

- Fixed #23: replace `struct stat` symbols (thanks to Peter Kokot).

## 3.1.2

- Restored `libeio/xthread.h` in package.xml.

## 3.1.1

- Fixed #21: `eio_fallocate()` tests failed on Fedora 40.
- Updated the embedded libeio.

## 3.1.0

- Fixed #19: initialized new stack pointers introduced in PHP 8.3.0RC1+.
- Fixed #20: PECL package version.

## 3.0.0RC5

- Fixed `-Wformat-overflow` compilation errors.
- Fixed build on s390x arch (thanks to Andy Postnikov).

## 3.0.0RC4

- Fixed deprecated calls on PHP 8.1 (thanks to Remi Collet).

## 3.0.0RC3

- Fixed #13: libeio sometimes didn't auto-initialize.

## 3.0.0RC2

- Fixed: the PECL package was broken for PHP 8.

## 3.0.0RC1

- Ported to PHP 8.

## 2.0.3

- Fixed build error in PHP 7.3.
- Removed ASAN from debug configuration.

## 2.0.2

- Fixed build error due to undefined `O_FSYNC` in musl libc.

## 2.0.1

- Fixed #7: [PHP7] segfault after shutdown.

## 2.0.0

- Fixed #6: PHP 7.1 build failed due to removed `zend_fcall_info.symbol_table`.

## 2.0.0RC3

- Fixed #4: linker issue on aarch64.

## 2.0.0RC2

- Removed extra memory allocations.
- PHP 7: `eio_custom` now throws an Exception in ZTS builds, because the Zend API is inaccessible from a custom thread in ZTS PHP 7 builds (there is a lack of Zend API to access TSRM thread-local data from a user thread).

## 2.0.0RC1

- Added PHP 7 support.
- Fixed uid/gid validation in `eio_fchown`.
- Fixed compilation issue on OS X 10.11.

## 1.2.6

- Fixed uid/gid validation in `eio_fchown`.
- Fixed compilation issue on OS X 10.11.

## 1.2.5

- Fixed Bitbucket issue #2: static build fails.
- Fixed build failure when EVENTFD was not available.

## 1.2.4

- Fixed Bitbucket issue #1: `eio_open` fails when file is owned by root.

## 1.2.3

- Fixed bug #65293: eio functions failed to process file descriptor equal to 0.

## 1.2.2

- Fixed: 3rd argument of `eio_custom` callback assigned as a resource instead of mixed.
- Fixed: `eio_custom` used an incorrect way of zval-to-zval copying.

## 1.2.1

- Fixed build failure without the sockets extension.
- Fixed `eio_write` failure when buffer length was less than size + offset.

## 1.2.0

- Added BSD support.
- Changed: internal event notification uses `pipe()` when `eventfd()` is not available.
- Removed some redundant code.
- Fixed: use libeio's `EIO_FALLOC_FL_KEEP_SIZE` instead of the system constant.

## 1.1.0

- Deprecated `eio_init()`; it will be removed in future. `pthread_atfork()` is used to re-init eio after a fork.
- Added phpt test for the fork support.
- Fixed: `tsrm_ls` was not initialized in `php_eio_init()`.
- Fixed: `EFD_NONBLOCK` and `EFD_CLOEXEC` were used despite the flags being unavailable before kernel 2.6.7. For old kernels `fcntl` is called.
- Set the `FD_CLOEXEC` flag for file descriptors opened with `eio_open()`.
- No need to copy zval when passing a request resource to `eio_grp_*()`.
- Fixed: `eio_grp_*()` didn't initialize eio automatically.

## 1.0.0

- Fixed: `eio_write()` wrote garbage when the `str` arg was passed by ref in a nested call with the `use` keyword.
- Fixed: `eio_write()` wrote garbage when the `str` arg was passed by value.
- Added `eio_init()`. It must be called before using Eio in a forked child process.
- Changed: cut the `st_` prefix in the result of `eio_*stat` for simplicity and better compatibility with `stat()`.
- Changed: cut the `f_` prefix in the result of `eio_*statvfs`.
- Added `eio_get_last_error()`.
- Added the `eio_req` resource as the third optional callback argument.
- Added phpt tests for the changes since 0.5.0b.
- Removed warnings having little sense even in debug mode.
- No separation for callbacks.

## 0.5.0 (beta)

- libeio upgraded.
- Fixed bug #62392: `eio_*stat` functions didn't return `st_size`.
- Fixed: package.xml referenced some nonexistent files.
- Fixed: garbage in the result arg of the callbacks in case of error.
- Changed: warn about a failed op in the callback only when compiled with debug support.
- Added test for stat on nonexistent files.
- Added sockets support (`eio_sendfile`, `eio_readahead`, etc.), enabled by default.
- Added `eio_seek` function.
- Changed: numeric fd arguments are now mixed (stream, Socket, or number).

## 0.4.0 (beta)

- Changed: libeio is now embedded, no need to install it separately.
- Changed: function call info and call info cache used instead of zvals.
- Fixed: segmentation fault on 32-bit platforms with the default 4-bit LFS.

## 0.3.1 (beta)

- Changed: `eio_get_event_stream` returns a stream resource.
- Fixed: package dependencies didn't allow `pecl install eio-alpha`.
- Fixed: redundant MS/NETWARE platform checks.

## 0.3.0 (alpha)

- Changed: the mutex switch used in communications with libeio was replaced with eventfd; the same descriptor is exported to userspace to bind with existing event loops.
- Added `eio_get_eventfd` function to get the eventfd descriptor.
- Added header checks in configuration.
- Changed: libeio initialization moved to the MINIT phase, but unfinished requests are still handled in the RSHUTDOWN phase.
- Simplified libeio callbacks.

## 0.2.0 (alpha)

- Fixed: heavy bulk requests damaged the event loop, causing high CPU consumption.
- Fixed: in a threaded SAPI libeio in a single process might initialize the mutex multiple times.
- Changed: switch from semaphore to mutex.
- Dropped `--with-eio-shm*` configuration options.

## 0.1.1 (alpha)

- Fixed: `eio_poll` had been out of sync with the IPC.
- Fixed pathnames in tests.
- Dropped preprocessor checks on non-UNIX platforms, i.e. no plans to support non-UNIX.

## 0.1.0 (alpha)

- Refactored some tests.
- Request types have readable values in error messages.
- `errno` replaced with a meaningful string in error messages.
- Fixed: `eio_chown` denied uid -1 value.
- Fixed message about required libeio.

## 0.0.1 (alpha)

- Changed: shared memory permissions are set to 0660 by default.
- Removed redundant code and comments.
- Added tests: `eio_read_basic.phpt`, `eio_stat_basic.phpt`.
- Fixed the package XML.
- Fixed docs.

## 0.0.1dev

- Initial development release.
