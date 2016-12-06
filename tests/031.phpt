--TEST--
write_to_file can set options
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $image = vips_image_new_from_file($filename)["out"];
  $output_filename1 = dirname(__FILE__) . "/x.jpg";
  $output_filename2 = dirname(__FILE__) . "/y.jpg";

  vips_image_write_to_file($image, $output_filename1, ["Q" => 20]);
  vips_image_write_to_file($image, $output_filename2, ["Q" => 90]);

  $buffer1 = file_get_contents($output_filename1);
  $buffer2 = file_get_contents($output_filename2);

  if (strlen($buffer1) < strlen($buffer2)) {
	echo "pass";
  }
?>
--EXPECT--
pass
--CLEAN--
<?php
  $output_filename = dirname(__FILE__) . "/x.jpg";
  unlink($output_filename);
  $output_filename = dirname(__FILE__) . "/y.jpg";
  unlink($output_filename);
?>

