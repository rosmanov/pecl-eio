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

if (class_exists('ArgumentCountError')) { // PHP 8+
    try {
        eio_chown($temp_filename);
    } catch (ArgumentCountError $e) {
        trigger_error($e->getMessage(), E_USER_WARNING);
    }
} else {
    eio_chown($temp_filename);
}


//Fatal error: Uncaught ArgumentCountError: eio_chown() expects at least 2 arguments, 1 given in /home/ruslan/projects/pecl/pecl-eio/tests/eio_chown_variation.phpt:23
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

%Aeio_chown() expects at least 2 %s, 1 given%A

Warning: eio_chown(): invalid uid and/or gid in %a
int(0)
