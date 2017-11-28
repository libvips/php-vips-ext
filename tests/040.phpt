--TEST--
can get info about cache and concurrency
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php
  $return = vips_cache_get_max_mem();

  if (is_int($return)) {
    echo "pass vips_cache_get_max_mem\n";
  }

  $return = vips_cache_get_max_files();

  if (is_int($return)) {
    echo "pass vips_cache_get_max_files\n";
  }

  $return = vips_cache_get_max();

  if (is_int($return)) {
    echo "pass vips_cache_get_max\n";
  }

  $return = vips_cache_get_size();

  if (is_int($return)) {
    echo "pass vips_cache_get_size\n";
  }

  $return = vips_concurrency_get();

  if (is_int($return)) {
    echo "pass vips_concurrency_get\n";
  }

?>
--EXPECT--
pass vips_cache_get_max_mem
pass vips_cache_get_max_files
pass vips_cache_get_max
pass vips_cache_get_size
pass vips_concurrency_get
