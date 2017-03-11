--TEST--
foreign_find_load works
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";

  $loader = vips_foreign_find_load($filename);

  if ($loader == "VipsForeignLoadJpegFile") { 
	echo "pass";
  }
?>
--EXPECT--
pass
