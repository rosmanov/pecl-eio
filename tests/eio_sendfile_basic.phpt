--TEST--
Check for eio_sendfile function basic behaviour
--FILE--
<?php 

function my_cb($data, $result) {
	var_dump($data);
	var_dump($result);
}

$tmp_file_from = sprintf("/tmp/tmp_%s", uniqid());
$tmp_file_to = sprintf("/tmp/tmp_%s", uniqid());

@unlink($tmp_file_from);
@unlink($tmp_file_to);

$fp_from = fopen($tmp_file_from, 'w');
fwrite($fp_from, '123456789');
fclose($fp_from);
$fp_from = fopen($tmp_file_from, 'r');

$fp_to = fopen($tmp_file_to, 'w');


eio_sendfile($fp_to, $fp_from, 0, 3, 0, 'my_cb', 'sendfile_data');
eio_event_loop();

fclose($fp_from);
fclose($fp_to);

@unlink($tmp_file_from);
@unlink($tmp_file_to);
?>
--EXPECT--
string(13) "sendfile_data"
int(3)
