--TEST--
Check for eio_get_last_error function
--FILE--
<?php 
$temp_filename = "eio-temp-file-nonexistent.tmp";

eio_open($temp_filename, NULL, NULL, EIO_PRI_DEFAULT, 
	function ($data, $result, $req) {
		if ($result < 0) {
			var_dump(eio_get_last_error($req));
		}
	});
eio_event_loop();
?>
--EXPECT--
string(25) "No such file or directory"
