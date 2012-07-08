--TEST--
Check for eio_mknod function basic behaviour
--FILE--
<?php 
$temp_filename = "eio-temp-fifo";

function my_mknod_callback($data, $result) {
	global $temp_filename;

	$s = stat($temp_filename);
	if ($result == 0 && $data == "mknod") {
		echo "eio_mknod_ok";
	} 

	if (file_exists($temp_filename)) {
		unlink($temp_filename);
	}
}


eio_mknod($temp_filename, EIO_S_IFIFO, 0, 0, "my_mknod_callback", "mknod");
eio_event_loop();
?>
--EXPECT--
eio_mknod_ok
