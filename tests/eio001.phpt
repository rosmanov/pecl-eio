--TEST--
Check for libeio presence
--SKIPIF--
<?php if (!extension_loaded("eio")) print "skip"; ?>
--FILE--
<?php 
echo "libeio extension is available";
?>
--EXPECT--
libeio extension is available
