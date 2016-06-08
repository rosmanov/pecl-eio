--TEST--
Check for eio fork support using pcntl
--SKIPIF--
<?php
if (!extension_loaded('pcntl')) {
	die('SKIP The pcntl extension is not loaded');
}
?>
--FILE--
<?php
$str      = str_repeat('1', 20);
$filename = '/tmp/tmp_file' .uniqid();
$filename2 = '/tmp/tmp_file2' .uniqid();
@unlink($filename);
@unlink($filename2);
touch($filename);
touch($filename2);

$pid = pcntl_fork();
if ($pid == -1) {
	die('could not fork');
} else if ($pid) {
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

	// we are the parent
	pcntl_wait($status); //Protect against Zombie children

} else {
	// we are the child
	eio_open($filename2, EIO_O_RDWR, NULL, EIO_PRI_DEFAULT, function($filename2, $fd) use ($str) {
		eio_write($fd, $str, strlen($str), 0, null, function($fd, $written) use ($str, $filename2) {
			var_dump([
				'written'  => $written,
				'strlen'   => strlen($str),
				'filesize' => filesize($filename2),
				'count'    => substr_count(file_get_contents($filename2), '1')
				]);
		}, $fd);
	}, $filename2);
	eio_event_loop();
}

@unlink($filename);
@unlink($filename2);
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
