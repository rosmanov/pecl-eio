--TEST--
Check for eio_mkdir function basic behaviour
--FILE--
<?php 
$temp_dirname = dirname(__FILE__) ."/eio-temp-dir";
@rmdir($temp_dirname);

function my_mkdir_callback($data, $result) {
	global $temp_dirname;
	if ($result == 0 && is_dir($temp_dirname) 
		&& !is_readable($temp_dirname)
		&& is_writable($temp_dirname)) {
		echo "eio_mkdir_ok";
	}
	if (file_exists($temp_dirname))
		rmdir($temp_dirname);
}

eio_mkdir($temp_dirname, 0300, 1, "my_mkdir_callback", NULL);
//eio_event_loop();
?>
--CLEAN--
<?php
?>
--EXPECT--
eio_mkdir_ok
