--TEST--
foreign_find_load_buffer works
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $buffer = file_get_contents($filename);

  $loader = vips_foreign_find_load_buffer($buffer);

  if ($loader == "VipsForeignLoadJpegBuffer") { 
	echo "pass";
  }
?>
--EXPECT--
pass
