--TEST--
can get version
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php 
  $version = vips_version();

  if (preg_match("/\d+\.\d+\.\d+/", $version)) { 
    echo "pass";
  }
?>
--EXPECT--
pass
