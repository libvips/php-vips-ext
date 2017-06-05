--TEST--
can make an interpolator
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $interp = vips_interpolate_new("bicubic");

  if ($interp != null) {
    echo "pass";
  }
?>
--EXPECT--
pass
