INSTALLATION OF EIO PECL EXTENSION
==================================

Currently this extension supports GNU/Linux and BSD platforms only. But you can
try it on any UNIX OS.


AUTOMATIC INSTALLATION
----------------------

To download and install eio automatically you need the following commands
for stable release:

	# pecl install eio

, and the following for beta release:

	# pecl install eio-beta

If you have the package archive, unpack it and run: 

	# pecl install package.xml

Note, these commands(started with '#') most likely need root priveleges.

Since version 0.4.0 beta libeio is embedded. So you don't need to install the
library separately.


MANUAL INSTALLATION
-------------------

Easy way, if you want just install it as root:

	# pecl install eio

If you want to tweak or debug it, checkout the project or download it as
archive. In the package directory run: 

	$ phpize
	$ ./configure --with-eio --enable-eio-sockets --enable-eio-debug
	$ make 

Optionally test the extension in CLI:

	$ make test

Do install with root priveleges:

	# make install

In php.ini, or some other configuration like
</usr/local/etc/php/conf.d/eio.ini> write:

	extension=eio.so

NOTES
-----

Since version 0.5.0 beta you're able to pass PHP streams, sockets(`socket_create`,
`socket_accept`), or numeric file descriptors to functions like `eio_sendfile`,
`eio_readahead`, `eio_read` etc.


AUTHOR
------

Ruslan Osmanov <osmanov@php.net>
<http://megagroup.ru/>
