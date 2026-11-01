--TEST--
Check that eio_cancel called twice on the same request doesn't use freed callback data
--FILE--
<?php
$req = eio_nop(EIO_PRI_DEFAULT, null, null);
eio_cancel($req);
eio_cancel($req);
eio_event_loop();
echo "done\n";
?>
--EXPECT--
done
