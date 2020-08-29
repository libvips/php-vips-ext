--TEST--
can set metadata
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $profilename = dirname(__FILE__) . "/images/sRGB.icc";
  $image = vips_image_new_from_file($filename)["out"];
  $data = file_get_contents($profilename);
  $output_filename = dirname(__FILE__) . "/x.tif";

  $image = vips_call("copy", $image)["out"];
  $result = vips_image_set_type($image, "VipsBlob", "icc-profile-data", $data);
  if ($result == 0) {
    echo "pass set_metadata\n";
  }

  vips_image_write_to_file($image, $output_filename);
  $new_image = vips_image_new_from_file($output_filename)["out"];
  if ($new_image != FALSE) {
    echo("pass reload\n");
  }

  $new_data = vips_image_get($image, "icc-profile-data")["out"];
  if ($new_data == $data) {
    echo("pass get_metadata\n");
  }
?>
--EXPECT--
pass set_metadata
pass reload
pass get_metadata
--CLEAN--
<?php
  $output_filename = dirname(__FILE__) . "/x.tif";
  unlink($output_filename);
?>

