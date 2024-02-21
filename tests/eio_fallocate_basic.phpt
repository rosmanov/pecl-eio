--TEST--
Check for eio_fallocate function basic behaviour
--SKIPIF--
<?php if (PHP_OS != "Linux") die("skip, only for linux"); ?>
--FILE--
<?php
error_reporting(0);

$temp_filename = "eio-temp-file.tmp";

function my_close_cb($data, $result) {
        global $temp_filename;

        echo 'close: ', var_export($result == 0, true), PHP_EOL;
        @unlink($temp_filename);
}

function my_falloc_cb($data, $result, $req) {
    $notImplemented = eio_get_last_error($req) === 'Function not implemented';
        echo "falloc: ", var_export($result == 0 || $notImplemented, true),  PHP_EOL;
}

function my_file_opened_callback($data, $result) {

        echo 'open: ', var_export($result > 0, true), PHP_EOL;
        if ($result > 0) {
                eio_fallocate($result, NULL, 0, 10, EIO_PRI_DEFAULT, "my_falloc_cb", NULL);
                eio_event_loop();

                eio_close($result, EIO_PRI_DEFAULT, "my_close_cb");
                eio_event_loop();
        }
}


eio_open($temp_filename, EIO_O_CREAT | EIO_O_RDWR, NULL, EIO_PRI_DEFAULT, "my_file_opened_callback", NULL);
eio_event_loop();
?>
--CLEAN--
--EXPECT--
open: true
falloc: true
close: true
