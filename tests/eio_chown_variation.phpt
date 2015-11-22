--TEST--
Check for eio_chown function basic behaviour
--SKIPIF--
<?php
if (!extension_loaded('posix')) {
	die('SKIP The posix extension is not loaded');
}
?>
--FILE--
<?php
ini_set('display_errors', 'On');
ini_set('log_errors', 'Off');
$temp_filename = "eio-temp-file.tmp";

touch($temp_filename);



function my_eio_chown_cb($data, $result) {
	var_dump($result);
}

eio_chown($temp_filename);
eio_event_loop();
eio_chown($temp_filename, -1, -1);
eio_event_loop();
eio_chown($temp_filename, posix_getuid(), -1, EIO_PRI_DEFAULT, "my_eio_chown_cb");
eio_event_loop();
?>
--CLEAN--
<?php
@unlink($temp_filename);
?>
--EXPECTF--

Warning: eio_chown() expects at least 2 parameters, 1 given %a

Warning: eio_chown(): invalid uid and/or gid in %a
int(0)
