--TEST--
Check for eio_stat, eio_lstat, eio_fstat functions' basic behaviour
--FILE--
<?php 
error_reporting(E_ERROR);
$tmp_filename = dirname(__FILE__) ."/eio-file.tmp";
touch($tmp_filename);

function my_res_cb($data, $result) {
	var_dump($data);
	var_dump($result['st_mtime']);
}

function my_open_cb($data, $result) {
	global $tmp_filename;

	eio_fstat($result, EIO_PRI_DEFAULT, "my_res_cb", "eio_fstat");
	eio_event_loop();

	eio_close($result);
	eio_event_loop();

	@unlink($tmp_filename);
}

eio_stat($tmp_filename, EIO_PRI_DEFAULT, "my_res_cb", "eio_stat");
eio_lstat($tmp_filename, EIO_PRI_DEFAULT, "my_res_cb", "eio_lstat");
eio_event_loop();

eio_open($tmp_filename, EIO_O_RDONLY, NULL, EIO_PRI_DEFAULT, "my_open_cb", "eio_open");
eio_event_loop();
?>
--EXPECTF--
string(%d) "eio_%stat"
int(%d)
string(%d) "eio_%stat"
int(%d)
string(%d) "eio_%stat"
int(%d)
