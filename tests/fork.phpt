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
			printf("w: %d l: %d f: %d c: %d\n",
				$written,
				strlen($str),
				filesize($filename),
				substr_count(file_get_contents($filename), '1'));
		}, $fd);
	}, $filename);
	eio_event_loop();

	// we are the parent
	pcntl_wait($status); //Protect against Zombie children

} else {
	// we are the child
	eio_open($filename2, EIO_O_RDWR, NULL, EIO_PRI_DEFAULT, function($filename2, $fd) use ($str) {
		eio_write($fd, $str, strlen($str), 0, null, function($fd, $written) use ($str, $filename2) {
			printf("w: %d l: %d f: %d c: %d\n",
				$written,
				strlen($str),
				filesize($filename2),
				substr_count(file_get_contents($filename2), '1'));
		}, $fd);
	}, $filename2);
	eio_event_loop();
}

@unlink($filename);
@unlink($filename2);
?>
--EXPECT--
w: 20 l: 20 f: 20 c: 20
w: 20 l: 20 f: 20 c: 20
