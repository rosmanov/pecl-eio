--TEST--
Check for eio_cancel function basic behaviour
--FILE--
<?php 
error_reporting(0);

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
--EXPECTF--
resource(%i) of type (EIO Request Descriptor)
my_nop
