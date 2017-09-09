--TEST--
can write to memory
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php
  $byte_array = array_fill(0, 200, 0);
  $image = vips_image_new_from_memory($byte_array, 20, 10, 1, 'uchar');
  $mem_arr = vips_image_write_to_memory($image);
  // var_dump($mem_arr);

  // FIXME: Doesn't work.
  if ($byte_array === $mem_arr) {
    echo "pass";
  }
?>
--EXPECT--
pass
