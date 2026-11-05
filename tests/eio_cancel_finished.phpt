--TEST--
Check that eio_cancel called on an already finished request is safe
--FILE--
<?php
function my_nop_cb($data, $result) {
	echo "my_nop\n";
}

$req = eio_nop(EIO_PRI_DEFAULT, "my_nop_cb", NULL);
eio_event_loop(); // runs and finishes the request
eio_cancel($req);
eio_cancel($req);
eio_event_loop();
echo "done\n";
?>
--EXPECT--
my_nop
done
