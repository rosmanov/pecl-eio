--TEST--
Check for eio_rename function basic behaviour
--FILE--
<?php 
$filename = dirname(__FILE__)."/eio-temp-file.dat";
touch($filename);
$new_filename = dirname(__FILE__)."/eio-temp-file-new.dat";


function my_rename_cb($data, $result) {
	global $filename, $new_filename;

	if ($result == 0 && !file_exists($filename) && file_exists($new_filename)) {
		@unlink($new_filename);
		echo "eio_rename_ok";
	} else {
		@unlink($filename);
	}
}


eio_rename($filename, $new_filename, EIO_PRI_DEFAULT, "my_rename_cb", $filename);
eio_event_loop();
?>
--CLEAN--
<?php
?>
--EXPECT--
eio_rename_ok
