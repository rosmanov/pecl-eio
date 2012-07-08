--TEST--
Check for eio_open function basic behaviour
--SKIPIF--
--FILE--
<?php 
error_reporting(0);

$temp_filename = "eio-temp-file.tmp";

function my_close_cb($data, $result) {
	global $temp_filename;

	var_dump($result == 0);
	@unlink($temp_filename);
}

function my_file_opened_callback($data, $result) {
	var_dump($result > 0);
	if ($result > 0) {
		eio_close($result, EIO_PRI_DEFAULT, "my_close_cb");
		eio_event_loop();
	}
}


eio_open($temp_filename, EIO_O_CREAT, NULL, EIO_PRI_DEFAULT, "my_file_opened_callback", NULL);
eio_event_loop();

?>
--EXPECT--
bool(true)
bool(true)
