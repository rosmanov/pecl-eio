--TEST--
Check for eio_chmod function basic behaviour
--FILE--
<?php 
$temp_filename = "eio-temp-file.tmp";

touch($temp_filename);

function my_chmod_callback($data, $result) {
	global $temp_filename;
	if ($result == 0 && !is_readable($temp_filename) && is_writable($temp_filename)) {
		echo "eio_chmod_ok";
	}
	@unlink($temp_filename);
}

eio_chmod($temp_filename, 0200, EIO_PRI_DEFAULT, "my_chmod_callback");
eio_event_loop();
?>
--CLEAN--
<?php
?>
--EXPECT--
eio_chmod_ok
