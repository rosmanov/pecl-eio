--TEST--
Check for eio_unlink function basic behaviour
--FILE--
<?php 
$temp_filename = "eio-temp.tmp";

$fp = fopen($temp_filename, "w");
fwrite($fp, "a");
fclose($fp);

function my_unlink_callback($data, $result) {
	global $temp_filename;

	if ($result == 0 && !file_exists($temp_filename)) {
		echo "eio_unlink";
	} 		
	@unlink($temp_filename);	
}



eio_unlink($temp_filename, EIO_PRI_DEFAULT, "my_unlink_callback");
eio_event_loop();
?>
--CLEAN--
<?php
?>
--EXPECT--
eio_unlink
