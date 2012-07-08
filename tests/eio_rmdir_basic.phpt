--TEST--
Check for eio_rmdir function basic behaviour
--FILE--
<?php 
$temp_dirname = "eio-temp-dir";

mkdir($temp_dirname);

function my_rmdir_callback($data, $result) {
	global $temp_dirname;

	if ($result == 0 && !file_exists($temp_dirname)) {
		echo "eio_rmdir_ok";
	} else if (file_exists($temp_dirname)) {
		rmdir($temp_dirname);
	}
}



eio_rmdir($temp_dirname, EIO_PRI_DEFAULT, "my_rmdir_callback");
eio_event_loop();
?>
--CLEAN--
<?php
?>
--EXPECT--
eio_rmdir_ok
