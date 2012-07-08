--TEST--
Check for eio_seek function basic behaviour
--SKIPIF--
--FILE--
<?php 
ini_set('display_errors', 'On');
ini_set('log_errors', 'Off');

$temp_filename = "eio-temp-file.tmp";
$fp = fopen($temp_filename, "w+");
fwrite($fp, "1234567890");

function my_seek_cb($fp, $result) {
	var_dump($result);
	if ($result == 0) {
		var_dump($fp);
		if (is_resource($fp)) {
			var_dump(fread($fp, 32));
		}
	}
}

if (!eio_seek($fp, 6, EIO_SEEK_SET, 0, 'my_seek_cb', $fp)) {
	die("Failed to eio_seek");
}
eio_event_loop();
if (!eio_seek($fp, -6, EIO_SEEK_END, 0, 'my_seek_cb', $fp)) {
	die("Failed to eio_seek");
}
eio_event_loop();
if (!eio_seek($fp, -100, EIO_SEEK_CUR, 0, 'my_seek_cb', $fp)) {
	die("Failed to eio_seek");
}
eio_event_loop();

fclose($fp);
@unlink($temp_filename);
?>
--EXPECTF--
int(0)
resource(5) of type (stream)
string(4) "7890"
int(0)
resource(5) of type (stream)
string(6) "567890"
int(-1)
