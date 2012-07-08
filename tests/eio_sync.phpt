--TEST--
Check for eio_sync, eio_fsync, eio_fdatasync functions' basic behaviour
--FILE--
<?php 

error_reporting(0);

$temp_filename = "eio-temp-file.tmp";

function my_close_cb($data, $result) {
	global $temp_filename;

	var_dump($result == 0);

	@unlink($temp_filename);
}

function my_fdatasync_cb($data, $result) {
	var_dump($result == 0);
}

function my_fsync_cb($data, $result) {
	var_dump($result == 0);
}

function my_sync_cb($data, $result) {
	var_dump($result == 0);
}

function my_file_opened_callback($data, $result) {
	var_dump($result > 0);
	if ($result > 0) {
		eio_fdatasync ($result, 0, "my_fdatasync_cb", "fdatasync");
		eio_fsync ($result, 0, "my_fsync_cb", "fsync");
		eio_sync (0, "my_sync_cb", "sync");
		eio_event_loop();

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
bool(true)
bool(true)
bool(true)
