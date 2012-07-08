--TEST--
Check for eio_open function error behaviour
--SKIPIF--
--FILE--
<?php 
error_reporting(0);

$temp_filename = "eio-temp-file-nonexistant.tmp";
function my_file_opened_callback($data, $result) {
	if ($result < 0) {
		echo "eio_open_ok";
	}
}


$req = eio_open($temp_filename, NULL, NULL, EIO_PRI_DEFAULT, "my_file_opened_callback", NULL);
eio_event_loop();
if (!$req) {
	echo "eio_open_ok";
}

error_reporting($old_error_reporting);
?>
--CLEAN--
<?php
?>
--EXPECT--
eio_open_ok
