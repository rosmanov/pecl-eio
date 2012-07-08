--TEST--
Check for eio_mkdir function basic behaviour
--FILE--
<?php 
$temp_dirname = dirname(__FILE__) ."/eio-temp-dir";
if (file_exists($temp_dirname)) {
	rmdir($temp_dirname);
}

function my_mkdir_callback($data, $result) {
	global $temp_dirname;
	if ($result == 0 && is_dir($temp_dirname) 
		&& fileperms($temp_dirname) & 0300) {
			echo "eio_mkdir_ok";
		} else {
			echo "eio_mkdir_failed";
		}
	if (file_exists($temp_dirname))
		rmdir($temp_dirname);
}


eio_mkdir($temp_dirname, 0300, 1, "my_mkdir_callback", NULL);
eio_event_loop();
?>
--EXPECT--
eio_mkdir_ok
