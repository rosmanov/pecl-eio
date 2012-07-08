--TEST--
Check for eio_sendfile function work with sockets
--SKIPIF--
<?php
if (!extension_loaded('sockets')) {
	die('SKIP The sockets extension is not loaded');
}
?>
--FILE--
<?php 
ini_set('display_errors', 'On');
ini_set('log_errors', 'Off');

function my_cb($socket, $result) {
	var_dump($socket);
	var_dump($result);

	if ($result <= 0) {
		return;
	}

	$data = socket_read($socket, 10, PHP_BINARY_READ);
	var_dump($data);
}

$tmp_file = sprintf("/tmp/tmp_%s", uniqid());
$fp = fopen($tmp_file, 'w+');
$data = "ABCdef123";
var_dump($data);
fwrite($fp, $data);

$sock_path = sprintf("/tmp/%s.sock", uniqid());
if (file_exists($sock_path))
	die('Temporary socket already exists.');

/* Setup socket */
$server = socket_create(AF_UNIX, SOCK_STREAM, 0);
if (!$server) {
	die('Unable to create AF_UNIX socket [server]');
}
if (!socket_bind($server,  $sock_path)) {
	die("Unable to bind to $sock_path");
}
if (!socket_listen($server, 2)) {
	die('Unable to listen on socket');
}

/* Connect to socket */
$client = socket_create(AF_UNIX, SOCK_STREAM, 0);
if (!$client) {
	die('Unable to create AF_UNIX socket [client]');
}
if (!socket_connect($client, $sock_path)) {
	die('Unable to connect to server socket');
}

/* Accept socket connection */
$socket = socket_accept($server);
if (!$socket) {
	die('Unable to accept connection');
}


eio_sendfile($client, $fp, 0, 8, 0, 'my_cb', $socket);
eio_event_loop();

fclose($fp);
socket_close($client);
socket_close($socket);
socket_close($server);
@unlink($sock_path);
@unlink($tmp_file);
?>
--EXPECT--
string(9) "ABCdef123"
resource(8) of type (Socket)
int(8)
string(8) "ABCdef12"
