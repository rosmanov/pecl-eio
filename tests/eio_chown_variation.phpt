--TEST--
Check for eio_chown function basic behaviour
--FILE--
<?php 
$er = error_reporting(E_WARNING);
#ini_set('display_errors', 'Off');
$temp_filename = "eio-temp-file.tmp";

touch($temp_filename);

function my_eio_chown_cb($data, $result) {
	var_dump($result);
}

eio_chown($temp_filename);
eio_event_loop();
eio_chown($temp_filename, 0);
eio_event_loop();
eio_chown($temp_filename, -1, -1);
eio_event_loop();
eio_chown($temp_filename, -1, 1001, EIO_PRI_DEFAULT, "my_eio_chown_cb");
eio_event_loop();
?>
--CLEAN--
<?php
@unlink($temp_filename);
error_reporting($er);
?>
--EXPECTF--
Warning: eio_chown() expects at least 2 parameters, 1 given in %s on line %a
Warning: eio_event_loop(): Operation not permitted, eio_req result: -1, req type: %s in %a on line %a
Warning: eio_chown(): invalid uid and/or gid in %s on line %a
int(0)
