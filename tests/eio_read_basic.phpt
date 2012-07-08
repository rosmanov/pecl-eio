--TEST--
Check for eio_read function basic behaviour
--SKIPIF--
--FILE--
<?php 
error_reporting(0);

$temp_filename = "eio-temp-file.tmp";
$fp = fopen($temp_filename, "w");
fwrite($fp, "1234567890");
fclose($fp);

function my_read_cb($data, $result) {
	global $temp_filename;

	var_dump($result);

	eio_close($data);
	eio_event_loop();

	@unlink($temp_filename);
}

function my_file_opened_callback($data, $result) {
	if ($result > 0) {
		eio_read($result, 5, 2, EIO_PRI_DEFAULT, "my_read_cb", $result);
		eio_event_loop();
	} else {
		unlink($data);	
	}
}


eio_open($temp_filename, EIO_O_CREAT | EIO_O_RDWR, NULL, 
	EIO_PRI_DEFAULT, "my_file_opened_callback", $temp_filename);
eio_event_loop();
?>
--EXPECT--
string(5) "34567"
