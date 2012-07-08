--TEST--
Check for eio_utime function basic behaviour
--FILE--
<?php 
$temp_filename = "eio-temp.tmp";

touch($temp_filename);

function my_utime_callback($data, $result) {
	global $temp_filename;

	$s = stat($temp_filename);

	if ($result == 0 && $s['atime'] == '1317665072' && $s['mtime'] == '1317665073') {
		echo "eio_utime_ok";
	} 

	@unlink($temp_filename);
}


eio_utime($temp_filename, 1317665072, 1317665073, EIO_PRI_DEFAULT, "my_utime_callback");
eio_event_loop();
?>
--CLEAN--
<?php
?>
--EXPECT--
eio_utime_ok
