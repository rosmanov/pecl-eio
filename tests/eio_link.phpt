--TEST--
Check for eio_link, eio_readlink, eio_symlink function basic behaviour
--FILE--
<?php 
$filename = dirname(__FILE__)."/symlink.dat";
touch($filename);
$link = dirname(__FILE__)."/symlink.link";
$hardlink = dirname(__FILE__)."/hardlink.link";

function my_hardlink_cb($data, $result) {
	global $link, $filename;
	var_dump(file_exists($data) && !is_link($data));
	@unlink($data);

	eio_symlink($filename, $link, EIO_PRI_DEFAULT, "my_symlink_cb", $link);
}

function my_symlink_cb($data, $result) {
	global $link, $filename;
	var_dump(file_exists($data) && is_link($data));

	if (!eio_readlink($data, EIO_PRI_DEFAULT, "my_readlink_cb", NULL)) {
		@unlink($link);
		@unlink($filename);
	}
}

function my_readlink_cb($data, $result) {
	global $filename, $link;
	var_dump($result);

	@unlink($link);
	@unlink($filename);
}


eio_link($filename, $hardlink, EIO_PRI_DEFAULT, "my_hardlink_cb", $hardlink);
eio_event_loop();
?>
--EXPECTF--
bool(true)
bool(true)
string(%d) "%ssymlink.dat"
