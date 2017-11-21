--TEST--
can write to memory
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php
  $binary_str = pack("C*", ...array_fill(0, 200, 0));
  $image = vips_image_new_from_memory($binary_str, 20, 10, 1, "uchar")["out"];
  $mem_str = vips_image_write_to_memory($image);

  if ($binary_str === $mem_str) {
    echo "pass";
  }
?>
--EXPECT--
pass
