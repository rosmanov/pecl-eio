Installation process is the same as for any PECL extension. There is a [special section on php.net](https://www.php.net/manual/en/install.pecl.php) describing the process. But just in case if you get lost, the following may clear up some points.

AUTOMATIC INSTALLATION
----------------------

In most cases, it should be enough to install the PEAR base system package (for `pecl` command) and then run:

	pecl install eio

The command above installs the latest stable release of `eio` uploaded to the PECL repository. In order to install a beta version, add `-beta` to the package name:

	pecl install eio-beta

If you have the package archive, you can unpack it and then run:

	pecl install package.xml

Note, the commands above require root privileges.

Since version 0.4.0 beta libeio is embedded so you don't need to install the library separately.

MANUAL INSTALLATION
-------------------

	phpize
	./configure --with-eio --enable-eio-sockets --enable-eio-debug
	make 

Running tests:

	make test
	make install # (requires root privileges)

In a PHP `.ini` configuration file, add the following directive:

	extension=eio.so

Note that `eio.so` should be loaded **after** `sockets.so`, so the `.ini` file for `eio.so` should go after the file with `extension=sockets.so` in alphabetical order. E.g.

```
20-sockets.ini
90-eio.ini
```
