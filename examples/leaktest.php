#!/usr/bin/env php
<?php

for($i = 0; $i < 2000; $i++) {
    echo "Loop $i ...\r";

    $x = vips_image_new_from_file($argv[1], ["access" => "sequential"])["out"];

    $width = vips_image_get($x, "width")["out"];
    $height = vips_image_get($x, "height")["out"];

    $x = vips_call("crop", $x, 100, 100, $width - 200, $height - 200)["out"];

    vips_image_write_to_file($x, $argv[2]);

    $x = NULL;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: expandtab sw=4 ts=4 fdm=marker
 * vim<600: expandtab sw=4 ts=4
 */

