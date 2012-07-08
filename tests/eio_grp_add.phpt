--TEST--
Check for eio_grp_add function basic behaviour
--FILE--
<?php 
$temp_filename = dirname(__FILE__) ."/eio-file.tmp";
$fp = fopen($temp_filename, "w");
fwrite($fp, "some data");
fclose($fp);

function my_grp_done($data, $result) {
	global $temp_filename;
	var_dump($result == 0); 
	@unlink($temp_filename);
}
function my_grp_file_opened_callback($data, $result) {
	global $my_file_fd, $grp;

	$my_file_fd = $result;

	var_dump($result > 0);

	$req = eio_read($my_file_fd, 4, 0, EIO_PRI_DEFAULT, "my_grp_file_read_callback");
	eio_grp_add($grp, $req);
}

function my_grp_file_read_callback($data, $result) {
	global $my_file_fd, $grp;

	var_dump($result);

	$req = eio_close($my_file_fd);
	eio_grp_add($grp, $req);

	eio_event_loop();
}


$grp = eio_grp("my_grp_done", "my_grp_data");
$req = eio_open($temp_filename, EIO_O_RDWR | EIO_O_APPEND , NULL, 0, "my_grp_file_opened_callback", NULL);
eio_grp_add($grp, $req);

var_dump($grp);

eio_event_loop();
?>
--CLEAN--
--EXPECTF--
resource(%d) of type (EIO Group Descriptor)
bool(true)
string(%d) "%s"
bool(true)
