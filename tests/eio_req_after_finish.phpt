--TEST--
Check that request and group resources are safe to use after libeio destroyed the request
--FILE--
<?php
$kept = null;

$req = eio_nop(EIO_PRI_DEFAULT, function ($data, $result, $r) use (&$kept) {
	$kept = $r; // outlives the request
	echo "cb\n";
});
$grp = eio_grp(function () {});
eio_event_loop();

// The requests are finished and destroyed by libeio at this point
eio_cancel($req);
eio_cancel($kept);
var_dump(eio_get_last_error($req));
eio_grp_add($grp, $req);
eio_grp_cancel($grp);
eio_grp_limit($grp, 1);

unset($req, $kept, $grp);
eio_event_loop();
echo "done\n";
?>
--EXPECT--
cb
bool(false)
done
