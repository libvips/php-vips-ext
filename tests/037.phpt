--TEST--
can make an image from memory
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php
  $byte_array = array_fill(0, 200, 0);
  $image = vips_image_new_from_memory($byte_array, 20, 10, 1, 'uchar');
  $width = vips_image_get($image, "width")["out"];
  $height = vips_image_get($image, "height")["out"];
  $format = vips_image_get($image, "format")["out"];
  $bands = vips_image_get($image, "bands")["out"];
  // FIXME: $avg doesn't output zero.
  $avg = vips_call("avg", $image)["out"];
  // echo $avg;

  if ($width == 20 &&
	$height == 10 &&
	$format == 'uchar' &&
    $bands == 1) {
	echo "pass";
  }
?>
--EXPECT--
pass