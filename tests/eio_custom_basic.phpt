--TEST--
Check for eio_custom function basic behaviour
--SKIPIF--
<?php
if (PHP_ZTS) {
	die("skip doesn't support ZTS");
}
?>
--FILE--
<?php
//error_reporting(0);

function my_custom_callback($data, $result, $req) {
	var_dump($data);
	var_dump(count($result));
	var_dump($result['data_modified']);
	var_dump($result['result']);
}

function my_custom($data) {
	var_dump($data);

	$result  = array(
		'result'	=> 1001,
		'data_modified' => "my custom data",
	);
	return $result;
}



$data = "my_custom_data";
$req = eio_custom("my_custom", 0, "my_custom_callback", $data);
var_dump($req);
eio_event_loop();
?>
--CLEAN--
--EXPECTF--
resource(%d) of type (EIO Request Descriptor)
string(14) "my_custom_data"
string(14) "my_custom_data"
int(2)
string(14) "my custom data"
int(1001)
