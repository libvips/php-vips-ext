--TEST--
write_to_array works
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $x = vips_image_new_from_file($filename)["out"];
  $bands = vips_image_get($x, "bands")["out"];
  $line = vips_call("crop", $x, 50, 50, 10, 1)["out"];
  $array = vips_image_write_to_array($line);
  # print_r($array);

  if (count($array) == 10 * $bands) {
    echo "pass array_size\n";
  }

  $pixel = vips_call("crop", $x, 50, 50, 1, 1)["out"];
  $r = vips_call("extract_band", $pixel, 0)["out"];
  $r = vips_call("avg", $r)["out"];
  $g = vips_call("extract_band", $pixel, 1)["out"];
  $g = vips_call("avg", $g)["out"];
  $b = vips_call("extract_band", $pixel, 2)["out"];
  $b = vips_call("avg", $b)["out"];
  # echo "r = " . $r . ", g = " . $g . ", b = " . $b . "\n";

  if ($array[0] == $r && $array[1] == $g && $array[2] == $b) {
    echo "pass pixel_value\n";
  }

?>
--EXPECT--
pass array_size
pass pixel_value
