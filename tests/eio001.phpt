--TEST--
Check for eio presence
--SKIPIF--
<?php if (!extension_loaded("eio")) print "skip"; ?>
--FILE--
<?php
echo "eio extension is available";
?>
--EXPECT--
eio extension is available
