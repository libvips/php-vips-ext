--TEST--
can call draw operations
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $image = vips_image_new_from_file($filename)["out"];

  $x = vips_call("draw_circle", $image, [255], 50, 50, 10, ["fill" => true])["image"];

  $pixel = vips_call("crop", $x, 50, 50, 1, 1)["out"];
  $r = vips_call("extract_band", $pixel, 0)["out"];
  $r = vips_call("avg", $r)["out"];
  $g = vips_call("extract_band", $pixel, 1)["out"];
  $g = vips_call("avg", $g)["out"];
  $b = vips_call("extract_band", $pixel, 2)["out"];
  $b = vips_call("avg", $b)["out"];

  if ($r == 255 &&
    $g == 255 &&
    $b == 255) {
    echo "pass";
  }
?>
--EXPECT--
pass
