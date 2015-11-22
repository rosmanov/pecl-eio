--TEST--
Check for eio_readdir function basic behaviour
--SKIPIF--
--FILE--
<?php

$dir = "./eio-unknown-dir";
$files = array ("$dir/abc", "$dir/def");
$data = "readdir_data";

mkdir($dir, 0700);
foreach ($files as $f) {
	touch($f);
}

eio_readdir($dir, EIO_READDIR_STAT_ORDER,0,
	function ($data, $result) {
		var_dump($data);
		var_dump($result);
	}, $data
);
eio_event_loop();

foreach ($files as $f) {
	unlink($f);
}

rmdir($dir);
?>
--EXPECTF--
string(12) "readdir_data"
array(2) {
  ["names"]=>
  array(2) {
    [0]=>
    string(3) "abc"
    [1]=>
    string(3) "def"
  }
  ["dents"]=>
  array(2) {
    [0]=>
    array(3) {
      ["name"]=>
      string(3) "abc"
      ["type"]=>
      int(%d)
      ["inode"]=>
      int(%d)
    }
    [1]=>
    array(3) {
      ["name"]=>
      string(3) "def"
      ["type"]=>
      int(%d)
      ["inode"]=>
      int(%d)
    }
  }
}
