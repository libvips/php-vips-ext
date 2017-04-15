--TEST--
can copy to memory
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $image1 = vips_image_new_from_file($filename)["out"];
  $image2 = vips_image_copy_memory($image1);

  $avg1 = vips_call("avg", $image1)["out"];
  $avg2 = vips_call("avg", $image2)["out"];

  if ($avg1 == $avg2) {
    echo "pass";
  }
?>
--EXPECT--
pass
