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
   | Notes                                                                |
   |                                                                      |
   | eio_mlock(), eio_mlockall(), eio_msync(), eio_mtouch() are not       |
   | implemented in this                                                  |
   | extension because of PHP's obvious limitations on user-sie memory    |
   | management.                                                          |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

/* IPC */
#include <poll.h>
#ifdef HAVE_SYS_EVENTFD_H
# include <stdint.h>
# include <sys/eventfd.h>
#endif

#include <string.h>		/* strerror() */

/* POSIX/UNIX API */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

/* PHP */
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_network.h"
#include "php_streams.h"

#if PHP_VERSION_ID >= 50301 && (HAVE_SOCKETS || defined(COMPILE_DL_SOCKETS))
# include "ext/sockets/php_sockets.h"
# define EIO_USE_SOCKETS
#endif


/* Internal */
#include "php_eio.h"
#include "eio_priv.h"

/* libeio */
#include "libeio/eio.h"
#include "libeio/eio.c"

static int le_eio_grp;
static int le_eio_req;

static pid_t php_eio_pid = 0;
static php_eio_pipe_t php_eio_pipe;

static const zend_module_dep eio_deps[] = {
	ZEND_MOD_OPTIONAL("sockets")
	{NULL, NULL, NULL}
};

/* {{{ eio_module_entry */
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

#define EIO_CB_SET_FIELD(n, v) \
	if (v) { \
		zval_add_ref(&v); \
	} else { \
		ALLOC_INIT_ZVAL(v); /* make new IS_NULL zval */ \
	} \
	eio_cb->n = v;

#define EIO_REQ_WARN_INVALID_CB() \
	php_error_docref(NULL TSRMLS_CC, E_WARNING, \
		"'%s' is not a valid callback", func_name);
#define EIO_REQ_CB_INIT(cb_type) \
	cb_type *eio_cb = (cb_type*) req->data; \
	TSRMLS_FETCH_FROM_CTX(eio_cb ? eio_cb->thread_ctx : NULL); \
	zval retval; \
	char *func_name;
#define EIO_BUF_ZVAL_P(req) ((zval *)(EIO_BUF(req)))
#define php_eio_pipe(fd) pipe(fd)
#define php_eio_fd(epp) ((epp)->fd[0])


static int php_eio_fd_prepare(int fd)
{
	return (fcntl(fd, F_SETFL, O_NONBLOCK) || fcntl(fd, F_SETFD, FD_CLOEXEC));
}

/* {{{ php_eio_eventfd() */
#ifdef HAVE_EVENTFD
static int php_eio_eventfd(void)
{
# if defined(EFD_NONBLOCK) && defined(EFD_CLOEXEC)
	/* Save extra calls to fcntl() */
	return eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
# else
	php_eio_pipe_t *ep = &php_eio_pipe;
	int fd = eventfd(0, 0);

	if (fd >= 0) {
		php_eio_fd_prepare(ep->fd[0]);
	}

	return fd;
# endif
}
#else /* No eventfd */
# define php_eio_eventfd() -1
#endif
/* }}} */

/* {{{ php_eio_pipe_new() */
static int php_eio_pipe_new(void)
{
	php_eio_pipe_t *ep = &php_eio_pipe;

	ep->fd[0] = php_eio_eventfd();

	if (ep->fd[0] >= 0) {
		ep->fd[1] = ep->fd[0];
		ep->len   = 8;
	} else { /* No eventfd. Make pipe */
		if (php_eio_pipe(ep->fd)) {
			return -1;
		}

		if (php_eio_fd_prepare(ep->fd[0]) || php_eio_fd_prepare(ep->fd[1])) {
			close(ep->fd[0]);
			close(ep->fd[1]);
			return -1;
		}

		ep->len = 1;
	}

	return 0;
}
/* }}} */

/* {{{ php_eio_pipe_destroy() */
static void php_eio_pipe_destroy(void)
{
	close (php_eio_pipe.fd[0]);

	if (php_eio_pipe.fd[1] != php_eio_pipe.fd[0]) {
		close(php_eio_pipe.fd [1]);
	}

	php_eio_pipe.len = 0;
}
/* }}} */

/* {{{ php_eio_event_loop() */
static void php_eio_event_loop(void)
{
	while (eio_nreqs()) {
		struct pollfd pfd;
		pfd.fd = php_eio_fd(&php_eio_pipe);
		pfd.events = POLLIN;
		poll(&pfd, 1, -1);
		eio_poll();
	}

}
/* }}} */

/* {{{ php_eio_set_readdir_names */
static void php_eio_set_readdir_names(zval * z, const eio_req * req)
{
	int i, len;
	zval *names_array;
	char *names = EIO_BUF(req);

	MAKE_STD_ZVAL(names_array);
	array_init(names_array);

	for (i = 0; i < EIO_RESULT(req); ++i) {
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

/* {{{ php_eio_set_readdir_dents */
static void php_eio_set_readdir_dent_and_names(zval * z, const eio_req * req)
{
	zval *names_array, *dents_array;
	int i;
	char *names = EIO_BUF(req);
	struct eio_dirent *ents = (struct eio_dirent *) req->ptr1;

	/* $names_array = array();
	 * $dents_array = array(); */
	MAKE_STD_ZVAL(names_array);
	MAKE_STD_ZVAL(dents_array);
	array_init(names_array);
	array_init(dents_array);

	for (i = 0; i < EIO_RESULT(req); ++i) {
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
static inline void php_eio_free_eio_cb(php_eio_cb_t * eio_cb)
{
	if (eio_cb) {
		zval_ptr_dtor(&eio_cb->arg);

		efree(eio_cb->fcc);

		if (ZEND_FCI_INITIALIZED(*eio_cb->fci)) {
			zval_ptr_dtor(&eio_cb->fci->function_name);
#if PHP_VERSION_ID >= 50300
			if (eio_cb->fci->object_ptr) {
				zval_ptr_dtor(&eio_cb->fci->object_ptr);
			}
#endif
		}
		efree(eio_cb->fci);

		efree(eio_cb);
		eio_cb = NULL;
	}
}

/* }}} */

/* {{{ php_eio_free_eio_cb_custom
 * Free an instance of php_eio_cb_custom_t */
static inline void php_eio_free_eio_cb_custom(php_eio_cb_custom_t *eio_cb)
{
	if (!eio_cb) {
		return;
	}

	if (eio_cb->arg) {
		zval_ptr_dtor(&eio_cb->arg);
		eio_cb->arg = NULL;
	}

	if (eio_cb->fcc) {
		efree(eio_cb->fcc);
		eio_cb->fcc = NULL;
	}

	if (eio_cb->fci) {
		if (ZEND_FCI_INITIALIZED(*eio_cb->fci)) {
			zval_ptr_dtor(&eio_cb->fci->function_name);
#if PHP_VERSION_ID >= 50300
			if (eio_cb->fci->object_ptr) {
				zval_ptr_dtor(&eio_cb->fci->object_ptr);
			}
#endif
		}
		efree(eio_cb->fci);
		eio_cb->fci = NULL;
	}

	if (eio_cb->fcc_exec) {
		efree(eio_cb->fcc_exec);
		eio_cb->fcc_exec = NULL;
	}

	if (eio_cb->fci_exec) {
		if (ZEND_FCI_INITIALIZED(*eio_cb->fci_exec)) {
			zval_ptr_dtor(&eio_cb->fci_exec->function_name);
#if PHP_VERSION_ID >= 50300
			if (eio_cb->fci_exec->object_ptr) {
				zval_ptr_dtor(&eio_cb->fci_exec->object_ptr);
			}
#endif
		}
		efree(eio_cb->fci_exec);
		eio_cb->fci_exec = NULL;
	}

	efree(eio_cb);
	eio_cb = NULL;
}
/* }}} */

/* {{{ php_eio_new_eio_cb
 * Allocates memory for a new instance of php_eio_cb_t.
 * Returns pointer to the new instance */
static inline php_eio_cb_t * php_eio_new_eio_cb(zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr, zval * data TSRMLS_DC)
{
	php_eio_cb_t *eio_cb = safe_emalloc(1, sizeof(php_eio_cb_t), 0);
	eio_cb->fci          = safe_emalloc(1, sizeof(zend_fcall_info), 0);
	eio_cb->fcc          = safe_emalloc(1, sizeof(zend_fcall_info_cache), 0);

	memcpy(eio_cb->fci, fci_ptr, sizeof(zend_fcall_info));
	memcpy(eio_cb->fcc, fcc_ptr, sizeof(zend_fcall_info_cache));

	if (ZEND_FCI_INITIALIZED(*fci_ptr)) {
		/* Prevent auto-destruction of the zvals within */
		Z_ADDREF_P(eio_cb->fci->function_name);
#if PHP_VERSION_ID >= 50300
		if (eio_cb->fci->object_ptr) {
			Z_ADDREF_P(eio_cb->fci->object_ptr);
		}
#endif
	}

	EIO_CB_SET_FIELD(arg, data);

	TSRMLS_SET_CTX(eio_cb->thread_ctx);

	return eio_cb;
}
/* }}} */

/* {{{ php_eio_new_eio_cb_custom
 * Allocates memory for a new instance of php_eio_cb_custom_t
 * Returns pointer to the new instance */
static inline php_eio_cb_custom_t * php_eio_new_eio_cb_custom(zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr, zend_fcall_info * fci_exec_ptr, zend_fcall_info_cache * fcc_exec_ptr, zval * data TSRMLS_DC)
{
	php_eio_cb_custom_t *eio_cb = safe_emalloc(1, sizeof(php_eio_cb_custom_t), 0);
	eio_cb->fci                 = safe_emalloc(1, sizeof(zend_fcall_info), 0);
	eio_cb->fcc                 = safe_emalloc(1, sizeof(zend_fcall_info_cache), 0);
	eio_cb->fci_exec            = safe_emalloc(1, sizeof(zend_fcall_info), 0);
	eio_cb->fcc_exec            = safe_emalloc(1, sizeof(zend_fcall_info_cache), 0);

	memcpy(eio_cb->fci, fci_ptr, sizeof(zend_fcall_info));
	memcpy(eio_cb->fcc, fcc_ptr, sizeof(zend_fcall_info_cache));
	memcpy(eio_cb->fci_exec, fci_exec_ptr, sizeof(zend_fcall_info));
	memcpy(eio_cb->fcc_exec, fcc_exec_ptr, sizeof(zend_fcall_info_cache));

	if (ZEND_FCI_INITIALIZED(*eio_cb->fci)) {
		/* Prevent auto-destruction of the zvals within */
		Z_ADDREF_P(eio_cb->fci->function_name);
#if PHP_VERSION_ID >= 50300
		if (eio_cb->fci->object_ptr) {
			Z_ADDREF_P(eio_cb->fci->object_ptr);
		}
#endif
	}

	if (ZEND_FCI_INITIALIZED(*eio_cb->fci_exec)) {
		/* Prevent auto-destruction of the zvals within */
		Z_ADDREF_P(eio_cb->fci_exec->function_name);
#if PHP_VERSION_ID >= 50300
		if (eio_cb->fci_exec->object_ptr) {
			Z_ADDREF_P(eio_cb->fci_exec->object_ptr);
		}
#endif
	}

	EIO_CB_SET_FIELD(arg, data);

	TSRMLS_SET_CTX(eio_cb->thread_ctx);

	eio_cb->locked = 0;

	return eio_cb;
}
/* }}} */

/* {{{ php_eio_custom_execute
 * Is called by eio_custom(). Calls userspace function. */
static void php_eio_custom_execute(eio_req * req)
{
	zval **args[1];
	zval *key;
	zval *retval_ptr;
	php_eio_cb_custom_t *eio_cb = (php_eio_cb_custom_t *) req->data;
	TSRMLS_FETCH_FROM_CTX(eio_cb ? eio_cb->thread_ctx : NULL);

	if (!eio_cb) {
		return;
	}

	if (EIO_CANCELLED(req)) {
		php_eio_free_eio_cb_custom(eio_cb);
		return;
	}

	/* mutex? */
	eio_cb->locked = 1;

	EIO_RESULT(req) = -1;

	key = eio_cb->arg;
	args[0] = &key;
	zval_add_ref(&key);

	if (ZEND_FCI_INITIALIZED(*eio_cb->fci_exec)) {
		eio_cb->fci_exec->params         = args;
		eio_cb->fci_exec->retval_ptr_ptr = &retval_ptr;
		eio_cb->fci_exec->param_count    = 1;
		eio_cb->fci_exec->no_separation  = 1;

		if (zend_call_function(eio_cb->fci_exec, eio_cb->fcc_exec TSRMLS_CC) == SUCCESS
			&& retval_ptr) {
			zval **ppz = (zval **)&(EIO_BUF(req));
			MAKE_STD_ZVAL(*ppz);
			ZVAL_ZVAL(*ppz, retval_ptr, 1, 1);
			/*zval_ptr_dtor(&retval_ptr);*/

			/* Required for libeio */
			EIO_RESULT(req) = 0;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"An error occurred while invoking exec function");
		}
	}

	zval_ptr_dtor(&key);
}
/* }}} */

/* {{{ php_eio_res_cb_custom */
static int php_eio_res_cb_custom(eio_req * req)
{
	zval **args[2];
	zval *key1, *key2, *key3;
	php_eio_cb_custom_t *eio_cb = (php_eio_cb_custom_t *) req->data;
	TSRMLS_FETCH_FROM_CTX(eio_cb ? eio_cb->thread_ctx : NULL);
	zval *retval_ptr;

	if (!EIO_CB_CUSTOM_IS_LOCKED(eio_cb) && EIO_CANCELLED(req)) {
		php_eio_free_eio_cb_custom(eio_cb);
		return 0;
	}

	if (!eio_cb) {
		return 0;
	}

	/* set $data arg value */
	key1 = eio_cb->arg;
	args[0] = &key1;
	zval_add_ref(&key1);

	/* $result arg */
	if (EIO_BUF_ZVAL_P(req)) {
		key2 = (zval *) EIO_BUF(req);
		zval_add_ref(&key2);
	} else {
		ALLOC_INIT_ZVAL(key2);
	}
	args[1] = &key2;

	/* $req arg */
	if (EIO_BUF_ZVAL_P(req)) {
		key3 = EIO_BUF_ZVAL_P(req);
		zval_add_ref(&key3);
	} else {
		ALLOC_INIT_ZVAL(key3);
	}
	args[2] = &key3;

	eio_cb->fci->params = args;
	eio_cb->fci->retval_ptr_ptr = &retval_ptr;
	eio_cb->fci->param_count = 3;
	eio_cb->fci->no_separation = 1;

	if (zend_call_function(eio_cb->fci, eio_cb->fcc TSRMLS_CC) == SUCCESS
		&& retval_ptr) {
		zval_ptr_dtor(&retval_ptr);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"An error occurred while invoking the callback");
	}

	zval_ptr_dtor(&key1);
	zval_ptr_dtor(&key2);
	zval_ptr_dtor(&key3);

	if (EIO_BUF_ZVAL_P(req)) {
		{
			zval *tmp = EIO_BUF_ZVAL_P(req);
			zval_ptr_dtor(&tmp);
			/*efree(EIO_BUF_ZVAL_P(req));*/
		}
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
static int php_eio_res_cb(eio_req *req)
{
	zval **args[3];
	zval *key1, *key2, *key3;
	zval *retval_ptr;
	php_eio_cb_t *eio_cb = (php_eio_cb_t *) req->data;
	TSRMLS_FETCH_FROM_CTX(eio_cb ? eio_cb->thread_ctx : NULL);

	if (!eio_cb) {
		return 0;
	}

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

	/* $data arg */
	key1 = eio_cb->arg;
	args[0] = &key1;
	zval_add_ref(&key1);

	/* $result arg */
	MAKE_STD_ZVAL(key2);
	args[1] = &key2;

	/* $req arg */
	MAKE_STD_ZVAL(key3);
	args[2] = &key3;
	ZEND_REGISTER_RESOURCE(key3, req, le_eio_req);

	/* {{{ set $result arg value */

	/* WARNING. If this callback returns nonzero, eio will stop processing
	 * results(in eio_poll), and will return the value to it's caller */
	if (EIO_RESULT(req) < 0) {
		/*EIO_REQ_WARN_RESULT_ERROR();*/
		ZVAL_LONG(key2, EIO_RESULT(req));
	} else {
		switch (req->type) {
			case EIO_OPEN:
				PHP_EIO_SETFD_CLOEXEC(EIO_RESULT(req));
				ZVAL_LONG(key2, EIO_RESULT(req));
				break;
			case EIO_READ:
				/* EIO_BUF(req) is the buffer with read contents
				 * (size_t) req->size = length parameter value passed to eio_read()
				 * Since req is destroyed later, data stored in req should be
				 * duplicated */
				if (EIO_RESULT(req) != -1) {
					ZVAL_STRINGL(key2, EIO_BUF(req), req->size, 1);
				} else {
					ZVAL_NULL(key2);
				}
				break;

			case EIO_READLINK:
			case EIO_REALPATH:
				/* EIO_BUF(req) is NOT-null-terminated string of result
				 * EIO_RESULT(req) is the length of the string */
				ZVAL_STRINGL(key2, EIO_BUF(req), EIO_RESULT(req), 1);
				break;

			case EIO_STAT:
			case EIO_LSTAT:
			case EIO_FSTAT:			/* {{{ */
				/* EIO_STAT_BUF(req) is ptr to EIO_STRUCT_STAT structure */
				array_init(key2);

				add_assoc_long(key2, "dev", EIO_STAT_BUF(req)->st_dev);
				add_assoc_long(key2, "ino", EIO_STAT_BUF(req)->st_ino);
				add_assoc_long(key2, "mode", EIO_STAT_BUF(req)->st_mode);
				add_assoc_long(key2, "nlink", EIO_STAT_BUF(req)->st_nlink);
				add_assoc_long(key2, "uid", EIO_STAT_BUF(req)->st_uid);
				add_assoc_long(key2, "size", EIO_STAT_BUF(req)->st_size);
				add_assoc_long(key2, "gid", EIO_STAT_BUF(req)->st_gid);
#ifdef HAVE_ST_RDEV
				add_assoc_long(key2, "rdev", EIO_STAT_BUF(req)->st_rdev);
#else
				add_assoc_long(key2, "rdev", -1);
#endif

#ifdef HAVE_ST_BLKSIZE
				add_assoc_long(key2, "blksize", EIO_STAT_BUF(req)->st_blksize);
#else
				add_assoc_long(key2, "blksize", -1);
#endif
#ifdef HAVE_ST_BLOCKS
				add_assoc_long(key2, "blocks", EIO_STAT_BUF(req)->st_blocks);
#else
				add_assoc_long(key2, "blocks", -1);
#endif

				add_assoc_long(key2, "atime", EIO_STAT_BUF(req)->st_atime);
				add_assoc_long(key2, "mtime", EIO_STAT_BUF(req)->st_mtime);
				add_assoc_long(key2, "ctime", EIO_STAT_BUF(req)->st_ctime);
				break;
				/* }}} */

			case EIO_STATVFS:
			case EIO_FSTATVFS:			/* {{{ */
				/* EIO_STATVFS_BUF(req) is ptr to EIO_STRUCT_STATVFS structure */
				array_init(key2);

				add_assoc_long(key2, "bsize", EIO_STATVFS_BUF(req)->f_bsize);
				add_assoc_long(key2, "frsize", EIO_STATVFS_BUF(req)->f_frsize);
				add_assoc_long(key2, "blocks", EIO_STATVFS_BUF(req)->f_blocks);
				add_assoc_long(key2, "bfree", EIO_STATVFS_BUF(req)->f_bfree);
				add_assoc_long(key2, "bavail", EIO_STATVFS_BUF(req)->f_bavail);
				add_assoc_long(key2, "files", EIO_STATVFS_BUF(req)->f_files);
				add_assoc_long(key2, "ffree", EIO_STATVFS_BUF(req)->f_ffree);
				add_assoc_long(key2, "favail", EIO_STATVFS_BUF(req)->f_favail);
				add_assoc_long(key2, "fsid", EIO_STATVFS_BUF(req)->f_fsid);
				add_assoc_long(key2, "flag", EIO_STATVFS_BUF(req)->f_flag);
				add_assoc_long(key2, "namemax", EIO_STATVFS_BUF(req)->f_namemax);
				break;
				/* }}} */

			case EIO_READDIR:
				/* EIO_READDIR_* flags are in req->int1
				 *
				 * EIO_BUF(req), which is req->ptr2, contains null-terminated names
				 * These will be stored in $result['names'] as a vector */
				array_init(key2);

				if (req->int1 & (EIO_READDIR_DENTS | EIO_READDIR_DIRS_FIRST)) {
					/* fill $result['dents'] with array of struct eio_dirent like arrays
					 * fill $result['names'] with dir names */
					php_eio_set_readdir_dent_and_names(key2, req);
				} else {
					/* If none of flags chosen. Not a good option, since in
					 * such case we've no info about offsets within EIO_BUF(req) ptr
					 * fill $result['names'] with dir names */
					php_eio_set_readdir_names(key2, req);
				}

				break;

			case EIO_WRITE:
				if (EIO_BUF(req)) {
					efree(EIO_BUF(req));
					EIO_BUF(req) = NULL;
				}
				ZVAL_LONG(key2, EIO_RESULT(req));
				break;

			default:
				ZVAL_LONG(key2, EIO_RESULT(req));
		}
	}
	/* }}} */

	if (ZEND_FCI_INITIALIZED(*eio_cb->fci)) {
		eio_cb->fci->params         = args;
		eio_cb->fci->retval_ptr_ptr = &retval_ptr;
		eio_cb->fci->param_count    = 3;
		eio_cb->fci->no_separation  = 1;

		if (zend_call_function(eio_cb->fci, eio_cb->fcc TSRMLS_CC) == SUCCESS
			&& retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"An error occurred while invoking the callback");
		}
	}

	zval_ptr_dtor(&key1);
	zval_ptr_dtor(&key2);
	zval_ptr_dtor(&key3);

	php_eio_free_eio_cb(eio_cb);

	return 0;
}
/*}}}*/

/* {{{ php_eio_want_poll_callback
 * Is called when eio wants attention(ready to process further requests) */
static void php_eio_want_poll_callback(void)
{
#if HAVE_EVENTFD
	static uint64_t counter = 1;
#else
	static char counter[8];
#endif
	if (write(php_eio_pipe.fd[1], &counter, php_eio_pipe.len) < 0
			&& errno == EINVAL
			&& php_eio_pipe.len != 8) {
		write(php_eio_pipe.fd[1], &counter, (php_eio_pipe.len = 8));
	}
}
/* }}} */

/* {{{ php_eio_done_poll_callback
 * Is invoked when eio detects that all pending requests have been handled */
static void php_eio_done_poll_callback(void)
{
	char buf[9];
	read(php_eio_pipe.fd[0], buf, sizeof(buf));
}
/* }}} */

/* {{{ php_eio_zval_to_fd
 * Get numeric file descriptor from PHP stream or Socket resource */
static php_socket_t php_eio_zval_to_fd(zval **ppfd TSRMLS_DC)
{
	php_socket_t file_desc = -1;
	php_stream *stream;
#ifdef EIO_USE_SOCKETS
	php_socket *php_sock;
#endif

	if (Z_TYPE_PP(ppfd) == IS_RESOURCE) {
		/* PHP stream or PHP socket resource  */
		if (ZEND_FETCH_RESOURCE_NO_RETURN(stream, php_stream *, ppfd, -1, NULL, php_file_le_stream())) {
			/* PHP stream */
			if (php_stream_cast(stream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL,
						(void*)&file_desc, 1) != SUCCESS || file_desc < 0) {
				return -1;
			}
		} else {
			/* PHP socket resource */
#ifdef EIO_USE_SOCKETS
			if (ZEND_FETCH_RESOURCE_NO_RETURN(php_sock, php_socket *, ppfd, -1, NULL, php_sockets_le_socket())) {
				return php_sock->bsd_socket;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "either valid PHP stream or valid PHP socket resource expected");
			}
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "valid PHP stream resource expected");
#endif
			return -1;
		}
	} else if (Z_TYPE_PP(ppfd) == IS_LONG) {
		/* Numeric fd */
		file_desc = Z_LVAL_PP(ppfd);
		if (file_desc < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid file descriptor passed");
			return -1;
		}
	} else {
		/* Invalid fd */
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid file descriptor passed");
		return -1;
	}

	return file_desc;
}
/* }}} */

/* {{{ php_eio_init() */
static inline void php_eio_init(TSRMLS_D)
{
	pid_t cur_pid = getpid();

	if (php_eio_pid <= 0 || (php_eio_pid > 0 && cur_pid != php_eio_pid)) {
		/* Uninitialized or forked a process(which needs it's own eio pipe) */

		if (php_eio_pipe_new()) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR,
					"Failed creating internal pipe: %s", strerror(errno));
			return;
		}

		if (eio_init(php_eio_want_poll_callback, php_eio_done_poll_callback)) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR,
					"Failed initializing eio: %s", strerror(errno));
			return;
		}

		php_eio_pid = cur_pid;
	}
}
/* }}} */

/* {{{ php_eio_atfork_child()
 * Re-initialize eio and internal pipe at fork */
static void php_eio_atfork_child(void)
{
	TSRMLS_FETCH();
	php_eio_init(TSRMLS_C);
}
/* }}} */

#undef EIO_CB_SET_FIELD
#undef EIO_REQ_WARN_INVALID_CB
#undef EIO_REQ_CB_INIT
#undef EIO_BUF_ZVAL_P

/* }}} */

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(eio)
{
	/* Re-initialize eio and internal pipe at fork */
	X_THREAD_ATFORK(NULL, NULL, php_eio_atfork_child);

	le_eio_grp =
		zend_register_list_destructors_ex(NULL, NULL,
		PHP_EIO_GRP_DESCRIPTOR_NAME, module_number);
	le_eio_req =
		zend_register_list_destructors_ex(NULL, NULL,
		PHP_EIO_REQ_DESCRIPTOR_NAME, module_number);

	/* {{{ Constants */
#ifdef EIO_DEBUG
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_DEBUG);
#endif

	/* {{{ EIO_SEEK_* */
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_SEEK_SET);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_SEEK_CUR);
	EIO_REGISTER_LONG_EIO_CONSTANT(EIO_SEEK_END);
	/* }}} */

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
#ifdef O_FSYNC
	EIO_REGISTER_LONG_CONSTANT(EIO_O_FSYNC, O_FSYNC);
#endif
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

	EIO_REGISTER_LONG_CONSTANT(EIO_FALLOC_FL_KEEP_SIZE, EIO_FALLOC_FL_KEEP_SIZE);

	/* }}} */


	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(eio)
{
	if (php_eio_pipe.len) {
		php_eio_pipe_destroy();
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(eio)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION */
PHP_RSHUTDOWN_FUNCTION(eio)
{
	php_eio_event_loop();

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

/* {{{ API */

/* {{{ proto void eio_init(void)
 * Deprecated. We use X_THREAD_ATFORK() to re-initialize eio on fork, and call php_eio_init() silently.
 * So user doesn't need to call eio_init() anymore.
 * TODO Remove in future
 * Create eio pipe for current process and eio_init().
 * Should be called from userspace within child process if forked. */
PHP_FUNCTION(eio_init)
{
	php_eio_init(TSRMLS_C);
}
/* }}} */

/* {{{ proto string eio_get_last_error(resource req)
 * Get last error associated with the request resource */
PHP_FUNCTION(eio_get_last_error)
{
	zval *zreq;
	eio_req *req;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zreq) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(req, eio_req *, &zreq, -1,
		PHP_EIO_REQ_DESCRIPTOR_NAME, le_eio_req);

	RETURN_STRING(strerror(req->errorno), 1);
}
/* }}} */


/* {{{ POSIX API wrappers */

/* {{{ proto bool eio_event_loop(void);
 * Polls eio until all requests proceeded.
 * Returns TRUE on success, FALSE otherwise. */
PHP_FUNCTION(eio_event_loop)
{
	php_eio_event_loop();

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

/* {{{ proto resource eio_open(string path, int flags, int mode, int pri, callback callback = NULL[, mixed data = NULL]);
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
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slllf!|z!",
			&path, &path_len,
			&flags, &mode, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);
	if (!mode) {
		mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	}

	req = eio_open(path, flags, (mode_t) mode, pri, php_eio_res_cb, eio_cb);

	PHP_EIO_RET_REQ_RESOURCE(req, eio_open);
}

/* }}} */

/* {{{ proto resource eio_truncate(string path[, int offset = 0[, int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]]);
 * Truncate a file.
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_truncate)
{
	char *path;
	int path_len;
	long offset = 0;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|llf!z!",
			&path, &path_len, &offset, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	if (offset < 0) {
		offset = 0;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_truncate(path, offset, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_truncate);
}

/* }}} */

/* {{{ proto resource eio_chown(string path, int uid[, int gid = -1 [,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]]);
 * Change file/direcrory permissions. uid is user ID. gid is group ID. uid/gid
 * is ignored when it's value is -1. Returns request resource on success, otherwise FALSE */
PHP_FUNCTION(eio_chown)
{
	char *path;
	int path_len;
	long uid;
	long gid = -1;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|llf!z!",
			&path, &path_len, &uid, &gid, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	if (uid < 0 && gid < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid uid and/or gid");
		RETURN_FALSE;
	}

#ifdef EIO_DEBUG
	if (access(path, W_OK) != 0) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE,
			"path '%s' is not writable", path);
		RETURN_FALSE;
	}
#endif

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req =
		eio_chown(path, (uid_t) uid, (gid_t) gid, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_chown);
}

/* }}} */

/* {{{ proto bool eio_chmod(string path, int mode, [int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Change file/direcrory permissions system call. Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_chmod)
{
	char *path;
	int path_len;
	long mode;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|lf!z!",
			&path, &path_len, &mode, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_chmod(path, mode, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_chmod);
}

/* }}} */

/* {{{ proto resource eio_mkdir (string path, int mode, [int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Creates a directory. Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_mkdir)
{
	char *path;
	int path_len;
	long mode;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|lf!z!",
			&path, &path_len, &mode, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

#ifdef EIO_DEBUG
	if (access(path, F_OK) == 0) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE,
			"directory '%s' already exists", path);
		RETURN_FALSE;
	}
#endif

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_mkdir(path, mode, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_mkdir);
}
/* }}} */

/* {{{ proto resource eio_rmdir (string path[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Removes a directory. Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_rmdir)
{
	char *path;
	int path_len;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lf!z!",
			&path, &path_len, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

#ifdef EIO_DEBUG
	if (access(path, F_OK) != 0) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE,
			"directory '%s' is not accessible", path);
		RETURN_FALSE;
	}
#endif

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_rmdir(path, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_rmdir);
}
/* }}} */

/* {{{ proto resource eio_unlink(string path[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Removes a file. Returns TRUE on success, otherwise FALSE. */
PHP_FUNCTION(eio_unlink)
{
	char *path;
	int path_len;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lf!z!",
			&path, &path_len, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	if (access(path, F_OK) != 0) {
		/* Nothing to unlink */
		RETURN_TRUE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_unlink(path, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_unlink);
}
/* }}} */

/* {{{ proto bool eio_utime(string path, double atime, double mtime[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Change file last access and modification times.
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_utime)
{
	char *path;
	int path_len;
	double atime, mtime;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sd/d/|lf!z!",
			&path, &path_len,
			&atime, &mtime, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_utime(path, (eio_tstamp) atime, (eio_tstamp) mtime,
		pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_utime);
}

/* }}} */

/* {{{ proto resource eio_mknod(string path, int mode, int dev[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Create a special or ordinary file.
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_mknod)
{
	char *path;
	int path_len;
	unsigned long mode, dev;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll|lf!z!",
			&path, &path_len, &mode, &dev, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_mknod(path, (mode_t) mode, (dev_t) dev,
		pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_mknod);
}
/* }}} */

/* {{{ proto resource eio_link(string path, string new_path[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Create a hardlink for file
 * Returns request resource on success, otherwise FALSE. Always returns FALSE for Windows and Netware. */
PHP_FUNCTION(eio_link)
{
	char *path, *new_path;
	int path_len, new_path_len;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|lf!z!",
			&path, &path_len,
			&new_path, &new_path_len, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);
	EIO_CHECK_PATH_LEN(new_path, new_path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_link(path, new_path, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_link);
}
/* }}} */

/* {{{ proto resource eio_symlink(string path, string new_path[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Create a symlink for file
 * Returns resource on success, otherwise FALSE. Always returns FALSE for Windows and Netware. */
PHP_FUNCTION(eio_symlink)
{
	char *path, *new_path;
	int path_len, new_path_len;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|lf!z!",
			&path, &path_len,
			&new_path, &new_path_len, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);
	EIO_CHECK_PATH_LEN(new_path, new_path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_symlink(path, new_path, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_symlink);
}
/* }}} */

/* {{{ proto resource eio_rename(string path, string new_path[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Change the name or location of a file.
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_rename)
{
	char *path, *new_path;
	int path_len, new_path_len;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|lf!z!",
			&path, &path_len,
			&new_path, &new_path_len, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);
	EIO_CHECK_PATH_LEN(new_path, new_path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_rename(path, new_path, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_rename);
}
/* }}} */

/* {{{ proto resource eio_close(mixed fd[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Closes file. Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_close)
{
	zval *zfd;
	int fd;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|lf!z!",
			&zfd, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
#ifdef EIO_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
			"invalid file descriptor '%d'", fd);
#endif
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_close(fd, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_close);
}

/* }}} */

/* {{{ proto resource eio_sync([int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Commit buffer cache to disk.
 * Returns request resource on success, otherwise FALSE. Always returns FALSE for Windows and Netware. */
PHP_FUNCTION(eio_sync)
{
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lf!z!",
			&pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_sync(pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_sync);
}
/* }}} */

/* {{{ proto resource feio_fsync(mixed fd[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Synchronize a file's in-core state with storage device.
 * Returns request resource on success, otherwise FALSE. Always returns FALSE for Windows and Netware. */
PHP_FUNCTION(eio_fsync)
{
	zval *zfd;
	int fd;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|lf!z!",
			&zfd, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}
	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_fsync(fd, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_fsync);
}

/* }}} */

/* {{{ proto resource eio_datafsync(mixed fd[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Synchronize a file's in-core state with storage device.
 * Returns request resource on success, otherwise FALSE. Always returns FALSE for Windows and Netware. */
PHP_FUNCTION(eio_fdatasync)
{
	zval *zfd;
	int fd;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|lf!z!",
			&zfd, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}
	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_fdatasync(fd, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_fdatasync);
}
/* }}} */

/* {{{ proto resource eio_futime(mixed fd, double atime, double mtime[ ,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Change file last access and modification times by file descriptor fd.
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_futime)
{
	zval *zfd;
	int fd;
	double atime, mtime;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zd/d/|lf!z!",
			&zfd, &atime, &mtime, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_futime(fd, (eio_tstamp) atime, (eio_tstamp) mtime,
		pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_futime);
}
/* }}} */

/* {{{ proto resource eio_ftruncate(mixed fd[, int offset = 0[, int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]]);
 * Truncate a file. Returns resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_ftruncate)
{
	zval *zfd;
	int fd;
	long offset = 0;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|llf!z!",
			&zfd, &offset, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	if (offset < 0) {
		offset = 0;
	}
	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_ftruncate(fd, offset, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_ftruncate);
}
/* }}} */

/* {{{ proto resource eio_fchmod(mixed fd, int mode, [int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Change file permissions system call by file descriptor fd.
 * Returns resource on success, otherwise FALSE.
 * The function always returns FALSE for NETWARE and WINDOWS */
PHP_FUNCTION(eio_fchmod)
{
	zval *zfd;
	int fd;
	long mode;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl/|lf!z!",
			&zfd, &mode, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_fchmod(fd, mode, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_fchmod);
}
/* }}} */

/* {{{ proto resource eio_fchown(mixed fd, int uid[, int gid = -1 [,int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]]);
 * Change file/direcrory permissions by file descriptor. uid is user ID. gid is group ID. uid/gid
 * is ignored when it's value is -1.
 * Returns resource on success, otherwise FALSE.
 * The function always returns FALSE for NETWARE and WINDOWS */
PHP_FUNCTION(eio_fchown)
{
	zval *zfd;
	int fd;
	long uid = -1, gid = -1;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl/|l/lf!z!",
			&zfd, &uid, &gid, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	if (uid < 0 && gid < 0) {
#  ifdef EIO_DEBUG
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid uid and/or gid");
#  endif
		RETURN_FALSE;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_fchown(fd, (uid_t) uid, (gid_t) gid, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_fchown);
}
/* }}} */

/* {{{ proto bool eio_dup2(mixed fd, mixed fd2, [int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]);
 * Duplicate a file descriptor.
 * Returns TRUE on success, otherwise FALSE. */
PHP_FUNCTION(eio_dup2)
{
	zval *zfd, *zfd2;
	int fd, fd2;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|lf!z!",
			&zfd, &zfd2, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	fd2 = php_eio_zval_to_fd(&zfd2 TSRMLS_CC);
	if (fd < 0 || fd2 < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_dup2(fd, fd2, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_dup2);
}
/* }}} */

/* {{{ proto resource eio_read(mixed fd, int length, int offset, int pri, callback callback[, mixed data = NULL]);
 * Read from a file descriptor, fd, at a given offset.
 *
 * length specifies amount of bytes to read.
 *
 * offset is offset from the beginning of the file. Default: 0.
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_read)
{
	zval *zfd;
	int fd;
	long length = 0, offset = 0;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zlllf!|z!",
			&zfd, &length, &offset, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	/* Actually, second parameter is buffer for read contents.
	 * But eio allocates memory for it's eio_req->ptr2 internally,
	 * and passes it to the callback. The buffer with read contents will be
	 * available in callback as EIO_BUF(req). Thus, we don't need allocate
	 * memory ourselves here */
	req = eio_read(fd, 0, length, offset, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_read);
}
/* }}} */

/* {{{ proto resource eio_write(mixed fd, mixed str[, int length = NULL[, int offset = 0[, int pri = 0 [, callback callback = NULL[, mixed data = NULL]]]]]);
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
	zval *zbuf, *zfd;
	int num_bytes, fd;
	unsigned long length = 0, offset = 0;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|lllf!z!",
			&zfd, &zbuf, &length, &offset, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid file descriptor");
		RETURN_FALSE;
	}

	if (Z_TYPE_P(zbuf) != IS_STRING) {
		convert_to_string(zbuf);
	}

	if (Z_STRLEN_P(zbuf) < length) {
		length = Z_STRLEN_P(zbuf);
	}

	if (ZEND_NUM_ARGS() == 2 || length <= 0) {
		num_bytes = Z_STRLEN_P(zbuf);
	} else {
		num_bytes = length;
	}

	if (!num_bytes) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Nothing to do");
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_write(fd, Z_STRVAL_P(zbuf), num_bytes, offset,
		pri, php_eio_res_cb, eio_cb);
	if (!Z_ISREF_P(zbuf)) {
		/* gonna be destructed. Prevent it.
		 * We then have to efree EIO_BUF(req) in php_eio_res_cb */
		EIO_BUF(req) = estrndup(EIO_BUF(req), num_bytes);
	}
	PHP_EIO_RET_REQ_RESOURCE(req, eio_write);
}
/* }}} */

/* {{{ proto resource eio_readlink(string path, int pri, callback callback[, mixed data = NULL]);
 * Read value of a symbolic link.
 *
 * Returns TRUE on success, otherwise FALSE.
 * Always returns FALSE for Netware and Windows. */
PHP_FUNCTION(eio_readlink)
{
	char *path;
	int path_len;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slf!|z!",
			&path, &path_len, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_readlink(path, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_readlink);
}
/* }}} */

/* {{{ proto resource eio_realpath(string path, int pri, callback callback[, mixed data = NULL]);
 * Return the canonicalized absolute pathname in the second argument of the
 * event completion callback.
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_realpath)
{
	char *path;
	int path_len;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slf!|z!",
			&path, &path_len, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_realpath(path, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_realpath);
}
/* }}} */

/* {{{ proto resource eio_stat(string path, int pri, callback callback[, mixed data = NULL]);
 * Get file status
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_stat)
{
	char *path;
	int path_len;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slf!|z!",
			&path, &path_len, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_stat(path, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_stat);
}
/* }}} */

/* {{{ proto resource eio_lstat(string path, int pri, callback callback[, mixed data = NULL]);
 * Get file status
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_lstat)
{
	char *path;
	int path_len;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slf!|z!",
			&path, &path_len, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_lstat(path, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_lstat);
}
/* }}} */

/* {{{ proto resource eio_fstat(mixed fd, int pri, callback callback[, mixed data = NULL]);
 * Get file status

 * fd	file descriptor
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_fstat)
{
	zval *zfd;
	int fd;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zlf!|z!",
			&zfd, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_fstat(fd, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_fstat);
}
/* }}} */

/* {{{ proto resource eio_statvfs(string path, int pri, callback callback[, mixed data = NULL]);
 * Get file system statistics
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_statvfs)
{
	char *path;
	int path_len;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slf!|z!",
			&path, &path_len, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	EIO_CHECK_PATH_LEN(path, path_len);

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_statvfs(path, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_statvfs);
}
/* }}} */

/* {{{ proto resource eio_fstatvfs(mixed fd, int pri, callback callback[, mixed data = NULL]);
 * Get file system statistics
 *
 * fd	file descriptor
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_fstatvfs)
{
	zval *zfd;
	int fd;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zlf!|z!",
			&zfd, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_fstatvfs(fd, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_fstatvfs);
}
/* }}} */

/* }}} */


/* {{{ proto resource eio_readdir(string path, int flags, int pri, callback callback[, mixed data = NULL]);
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
	long flags;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sllf|z!",
			&path, &path_len, &flags, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	/* In current version of eio it causes SEGVAULT without the following */
	if (flags & (EIO_READDIR_DIRS_FIRST | EIO_READDIR_STAT_ORDER)) {
		flags |= EIO_READDIR_DENTS;
	}

	req = eio_readdir(path, flags, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_readdir);
}
/* }}} */


/* {{{ OS-SPECIFIC CALL WRAPPERS */

/* {{{ proto resource eio_sendfile(mixed out_fd, mixed in_fd, int offset, int length[, int pri[, callback callback[, mixed data = NULL]]]);
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_sendfile)
{
	zval *zout_fd, *zin_fd;

	php_socket_t out_fd, in_fd;
	long offset, length;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzll|lf!z!",
			&zout_fd, &zin_fd, &offset, &length,
			&pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	out_fd = php_eio_zval_to_fd(&zout_fd TSRMLS_CC);
	in_fd = php_eio_zval_to_fd(&zin_fd TSRMLS_CC);
	if (out_fd < 0 || in_fd < 0) {
		/* php_eio_zval_to_fd reports errors if necessary */
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_sendfile(out_fd, in_fd, offset, length,
		pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_sendfile);
}
/* }}} */

/* {{{ proto resource eio_readahead(mixed fd, int offset, int length[, int pri[, callback callback[, mixed data = NULL]]]);
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_readahead)
{
	zval *zfd;
	int fd;
	long offset, length;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zll|lf!z!",
			&zfd, &offset, &length, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_readahead(fd, offset, length, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_readahead);
}
/* }}} */


/* {{{ proto resource eio_seek(mixed fd, int offset, int whence[, int pri[, callback callback[, mixed data = NULL]]]);
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_seek)
{
	zval *zfd;
	int fd;
	unsigned long offset, whence;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zll|lf!z!",
			&zfd, &offset, &whence, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_seek(fd, offset, whence, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_seek);
}
/* }}} */

/* {{{ proto resource eio_syncfs(mixed fd[, int pri[, callback callback[, mixed data = NULL]]]);
 *
 * Returns resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_syncfs)
{
	zval *zfd;
	int fd;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|lf!z!",
			&zfd, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_syncfs(fd, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_syncfs);
}
/* }}} */

/* {{{ proto resource eio_sync_file_range(mixed fd, int offset, int nbytes, int flags[, int pri[, callback callback[, mixed data = NULL]]]);
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_sync_file_range)
{
	zval *zfd;
	int fd;
	unsigned long offset, nbytes, flags;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zlll|lf!z!",
			&zfd, &offset, &nbytes, &flags, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_sync_file_range(fd, offset, nbytes, flags,
		pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_sync_file_range);
}
/* }}} */

/* {{{ proto resource eio_fallocate(mixed fd, int mode, int offset, int length[, int pri[, callback callback[, mixed data = NULL]]]);
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
	zval *zfd;
	int fd;
	unsigned long mode = 0, offset = 0, length;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zlll|lf!z!",
			&zfd, &mode, &offset, &length, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	fd = php_eio_zval_to_fd(&zfd TSRMLS_CC);
	if (fd < 0) {
		RETURN_FALSE;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_fallocate(fd, mode, offset, length, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_fallocate);
}
/* }}} */

/* }}} */


/* {{{ EIO-SPECIFIC REQUESTS */

/* {{{ proto resource eio_custom(callback execute, int pri, callback callback[, mixed data = NULL]);
 * Execute custom user function specified by execute argument just like other eio_* calls.
 *
 * execute	User function that should match the following prototype:
 * callback execute(mixed data);
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
 */
PHP_FUNCTION(eio_custom)
{
	long pri                       = EIO_PRI_DEFAULT;
	zval *data                     = NULL;
	zend_fcall_info fci            = empty_fcall_info;
	zend_fcall_info_cache fcc      = empty_fcall_info_cache;
	zend_fcall_info fci_exec;
	zend_fcall_info_cache fcc_exec;
	php_eio_cb_custom_t *eio_cb;
	eio_req *req;
	PHP_EIO_IS_INIT();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "flf!|z!",
			&fci_exec, &fcc_exec, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	eio_cb = php_eio_new_eio_cb_custom(&fci, &fcc, &fci_exec, &fcc_exec, data TSRMLS_CC);

	req = eio_custom(php_eio_custom_execute,
		pri, php_eio_res_cb_custom, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_custom);
}
/* }}} */

/* {{{ proto resource eio_busy(int delay[, int pri=EIO_PRI_DEFAULT[, callback callback=NULL[, mixed data=NULL]]]);
 * Artificially increase load. Could be useful in tests, benchmarking.
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_busy)
{
	unsigned long delay;
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|lf!z!",
			&delay, &pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_busy(delay, pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_busy);
}
/* }}} */

/* {{{ proto resource eio_nop([int pri[, callback callback[, mixed data = NULL]]]);
 * Does nothing, except go through the whole request cycle.
 *
 * Returns request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_nop)
{
	PHP_EIO_INIT;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lf!z!",
			&pri, &fci, &fcc, &data) == FAILURE) {
		return;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_nop(pri, php_eio_res_cb, eio_cb);
	PHP_EIO_RET_REQ_RESOURCE(req, eio_nop);
}
/* }}} */

/* }}} */

/* {{{ proto void eio_cancel(resource req);
 * Cancels a request */
PHP_FUNCTION(eio_cancel)
{
	zval *zreq;
	eio_req *req;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zreq) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(req, eio_req *, &zreq, -1,
		PHP_EIO_REQ_DESCRIPTOR_NAME, le_eio_req);

	if (req->type != EIO_CUSTOM) {
		eio_cancel(req);
		php_eio_free_eio_cb(req->data);
	} else if (!EIO_CB_CUSTOM_IS_LOCKED((php_eio_cb_custom_t *) req->data)) {
		eio_cancel(req);
		php_eio_free_eio_cb_custom(req->data);
	}
}
/* }}} */


/* {{{ GROUPING AND LIMITING REQUESTS */

/* {{{ proto resource eio_grp(callback callback[, mixed data = NULL]);
 * Creates, submits and returns a group request resource.
 *
 * Returns group request resource on success, otherwise FALSE. */
PHP_FUNCTION(eio_grp)
{
	zval *data                = NULL;
	zend_fcall_info fci       = empty_fcall_info;
	zend_fcall_info_cache fcc = empty_fcall_info_cache;
	php_eio_cb_t *eio_cb;
	eio_req *req;
	PHP_EIO_IS_INIT();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f|z!",
			&fci, &fcc, &data) == FAILURE) {
		return;
	}

	eio_cb = php_eio_new_eio_cb(&fci, &fcc, data TSRMLS_CC);

	req = eio_grp(php_eio_res_cb, eio_cb);
	PHP_EIO_RET_IF_FAILED(req, eio_grp);

	ZEND_REGISTER_RESOURCE(return_value, req, le_eio_grp);
}
/* }}} */

/* {{{ proto void eio_grp_add(resource grp, resource req);
 * Adds a request to the request group. */
PHP_FUNCTION(eio_grp_add)
{
	zval *zgrp, *zreq;
	eio_req *grp, *req;
	PHP_EIO_IS_INIT();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr",
			&zgrp, &zreq) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(grp, eio_req *, &zgrp, -1,
		PHP_EIO_GRP_DESCRIPTOR_NAME, le_eio_grp);
	ZEND_FETCH_RESOURCE(req, eio_req *, &zreq, -1,
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
	PHP_EIO_IS_INIT();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl",
			&zgrp, &limit) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(grp, eio_req *, &zgrp, -1,
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
	PHP_EIO_IS_INIT();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zgrp) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(grp, eio_req *, &zgrp, -1,
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

	eio_set_max_poll_time((eio_tstamp) nseconds);
}

/* }}} */

/* Quickies for simple functions */
#define EIO_SET_INT_FUNCTION(eio_func) \
	PHP_FUNCTION(eio_func) \
	{ \
		unsigned long num; \
		\
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &num) == FAILURE) { \
			return; \
		} \
		eio_func((unsigned int)num); \
	}

#define EIO_GET_INT_FUNCTION(eio_func) \
    PHP_FUNCTION(eio_func) \
	{ \
		RETURN_LONG(eio_func()); \
	}

EIO_SET_INT_FUNCTION(eio_set_max_poll_reqs)
EIO_SET_INT_FUNCTION(eio_set_min_parallel)
EIO_SET_INT_FUNCTION(eio_set_max_parallel)
EIO_SET_INT_FUNCTION(eio_set_max_idle)

EIO_GET_INT_FUNCTION(eio_nthreads)
EIO_GET_INT_FUNCTION(eio_nreqs)
EIO_GET_INT_FUNCTION(eio_nready)
EIO_GET_INT_FUNCTION(eio_npending)

#undef EIO_SET_INT_FUNCTION
#undef EIO_GET_INT_FUNCTION
/* }}} */

/* {{{ proto resource eio_get_event_stream(void) */
PHP_FUNCTION(eio_get_event_stream)
{
	php_stream *stream = php_stream_fopen_from_fd(php_eio_fd(&php_eio_pipe), "r", NULL);
	if (stream) {
		php_stream_to_zval(stream, return_value);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
