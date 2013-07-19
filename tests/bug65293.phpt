--TEST--
Check for bug #65293
--FILE--
<?php 

eio_close(0, 0, function ($d, $r) {
	var_dump($r);
});
eio_event_loop();
?>
--EXPECT--
int(0)
