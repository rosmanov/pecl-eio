--TEST--
Check for eio_*stat functions' error behaviour
--FILE--
<?php 
ini_set('display_errors', 'On');
ini_set('log_errors', 'Off');
$tmp_filename= './tmpfile';
touch($tmp_filename);

function my_res_cb($data, $result) {
	var_dump($data);
	if (is_array($result)) {
		var_dump($result['mtime']);
	} else {
		var_dump($result);
	}
}
function my_open_cb($data, $result) {
	if ($result > 0) {
		eio_fstat($result, EIO_PRI_DEFAULT, "my_res_cb", "eio_fstat");
		eio_event_loop();

		eio_close($result);
		eio_event_loop();
	} else {
		var_dump($result);
	}
}


eio_stat($tmp_filename, EIO_PRI_DEFAULT, "my_res_cb", "eio_stat");
eio_lstat($tmp_filename, EIO_PRI_DEFAULT, "my_res_cb", "eio_lstat");
eio_event_loop();
eio_open($tmp_filename, EIO_O_RDONLY, NULL, EIO_PRI_DEFAULT, "my_open_cb", "eio_open");
eio_event_loop();

@unlink($tmp_filename);

eio_stat($tmp_filename, EIO_PRI_DEFAULT, "my_res_cb", "eio_stat");
eio_lstat($tmp_filename, EIO_PRI_DEFAULT, "my_res_cb", "eio_lstat");
eio_event_loop();
?>
--EXPECTF--
string(%d) "eio_%stat"
int(%d)
string(%d) "eio_%stat"
int(%d)
string(%d) "eio_%stat"
int(%d)
string(%d) "eio_%stat"
int(-1)
string(%d) "eio_%stat"
int(-1)
