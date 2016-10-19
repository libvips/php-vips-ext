--TEST--
enum fields are returned as strings
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $image = vips_image_new_from_file($filename)["out"];
  $interpretation = vips_image_get($image, "interpretation")["out"];
  if ($interpretation == "srgb") {
    echo("pass\n");
  }
?>
--EXPECT--
pass
