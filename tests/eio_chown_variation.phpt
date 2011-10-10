--TEST--
Check for eio_chown function basic behaviour
--SKIPIF--
<?php 
if(substr(PHP_OS, 0, 3) == "WIN")
	die("skip, not supported on Windows");
?>
--FILE--
<?php 
$er = error_reporting(E_WARNING);
$temp_filename = "eio-temp-file.tmp";

$fp = fopen($temp_filename, "w");
fwrite($fp, "a");
fclose($fp);

eio_chown($temp_filename);
eio_event_loop();
eio_chown($temp_filename, 0);
eio_event_loop();
eio_chown($temp_filename, 1001, 2000);
eio_event_loop();
?>
--CLEAN--
<?php
@unlink($temp_filename);
error_reporting($er);
?>
--EXPECTF--
Warning: eio_chown() expects at least 2 parameters, 1 given in %s on line %a
Warning: eio_chown(): invalid uid and/or gid in %s on line %a
Warning: eio_event_loop(): eio_req result: -1, %aerrno: %i in %s on line %d
