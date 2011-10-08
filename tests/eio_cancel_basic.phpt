--TEST--
Check for eio_cancel function basic behaviour
--FILE--
<?php 
$old_error_reporting = error_reporting(0);

function my_nop_cb($data, $result) {
	echo "my_nop";
}

$req = eio_nop(EIO_PRI_DEFAULT, "my_nop_cb", NULL);
var_dump($req);
eio_cancel($req);
eio_nop(EIO_PRI_DEFAULT, "my_nop_cb", NULL);
eio_event_loop();
?>
--CLEAN--
<?php
error_reporting($old_error_reporting);
?>
--EXPECTF--
resource(%i) of type (EIO Request Descriptor)
my_nop
