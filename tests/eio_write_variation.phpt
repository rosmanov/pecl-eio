--TEST--
Check for eio_write function behaviour with `use' keyword within a nested call
--FILE--
<?php
$str      = str_repeat('1', 20);
$filename = '/tmp/tmp_file' .uniqid();
@unlink($filename);
touch($filename);


eio_open($filename, EIO_O_RDWR, NULL, EIO_PRI_DEFAULT, function($filename, $fd) use ($str) {
	eio_write($fd, $str, strlen($str), 0, null, function($fd, $written) use ($str, $filename) {
		var_dump([
			'written'  => $written,
			'strlen'   => strlen($str),
			'filesize' => filesize($filename),
			'count'    => substr_count(file_get_contents($filename), '1')
			]);
	}, $fd);
}, $filename);
eio_event_loop();
?>
--EXPECT--
array(4) {
  ["written"]=>
  int(20)
  ["strlen"]=>
  int(20)
  ["filesize"]=>
  int(20)
  ["count"]=>
  int(20)
}
