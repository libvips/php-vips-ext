--TEST--
can enlarge with a bicubic interpolator
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $image1 = vips_image_new_from_file($filename)["out"];
  $interp = vips_interpolate_new("bicubic");

  $sz = vips_call("affine", $image1, [2, 0, 0, 2], ["interpolate" => $interp])["out"];

  $w1 = vips_image_get($image1, "width")["out"];
  $wr = vips_image_get($sz, "width")["out"];

  if ($w1 * 2 == $wr) {
    echo "pass";
  }
?>
--EXPECT--
pass
