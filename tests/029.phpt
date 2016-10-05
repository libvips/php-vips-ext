--TEST--
can get error messages
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $image = vips_image_new_from_file($filename)["out"];

  $rg = vips_call("extract_band", $image, 0, ["n" => 2])["out"];

  # this should error out since it's 2 band image + 3 band image
  $err = vips_call("add", $image, $rg);
  $msg = vips_error_buffer();

  if ($err == -1 &&
    $msg == "add: not one band or 3 bands\n") {
    echo "pass";
  }
?>
--EXPECT--
pass
