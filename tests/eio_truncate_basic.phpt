--TEST--
Check for eio_truncate function basic behaviour
--SKIPIF--
--FILE--
<?php 
error_reporting(0);

$temp_filename = "eio-temp-file.tmp";

$fp = fopen($temp_filename, "w");
for ($i = 0; $i < 10; $i++) {
	fwrite($fp, "a");
}
fclose($fp);

function my_file_truncated_callback($data, $result) {
	global $temp_filename;

	if ($result >= 0 && filesize($temp_filename) == 5) {
		echo "eio_truncate_ok";
	}
	@unlink($temp_filename);
}


$req = eio_truncate($temp_filename, 5, EIO_PRI_DEFAULT, "my_file_truncated_callback");
eio_event_loop();
?>
--CLEAN--
<?php
@unlink($temp_filename);
?>
--EXPECT--
eio_truncate_ok
