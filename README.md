# Low-level PHP binding for libvips 

This extension lets you use the libvips image processing library from PHP. It is
intentionally very low-level. Modules such as
https://github.com/jcupitt/php-vips try to layer a nice API on
top of this. 

libvips is fast and it can work without needing to have the 
entire image loaded into memory. Programs that use libvips don't
manipulate images directly, instead they create pipelines of image processing
operations starting from a source image. When the end of the pipe is connected
to a destination, the whole pipeline executes at once, streaming the image
in parallel from source to destination in a set of small fragments. 

See the [benchmarks at the official libvips
website](http://www.vips.ecs.soton.ac.uk/index.php?title=Speed_and_Memory_Use).
There's a handy blog post explaining [how libvips opens
files](http://libvips.blogspot.co.uk/2012/06/how-libvips-opens-file.html)
which gives some more background.

### Example

```php
#!/usr/bin/env php
<?php
if (!extension_loaded("vips")) {
    dl('vips.' . PHP_SHLIB_SUFFIX);
}

$x = vips_image_new_from_file($argv[1])["out"];
$x = vips_call("invert", $x)["out"];
vips_image_write_to_file($x, $argv[2]);
?>
```

Almost all operations return an array of result values. Usually there is a
single result called `"out"`.

Use `vips_call()` to call any operation in the vips library. There are around 
around 300 operations available, see the vips docs for an
introduction:

http://www.vips.ecs.soton.ac.uk/supported/current/doc/html/libvips/

Arguments can be long, double, image, array of long, array of double or array
of image. The final argument to `vips_call()` is an array of operation options. 

### Preparation

PHP is normally built for speed and is missing a lot of debugging support you
need for extension development. For testing and dev, build your own php. 
I used 7.0.11 and configured with:

```
$ ./configure --prefix=/home/john/vips --enable-debug --enable-maintainer-zts \
    --enable-cgi --enable-cli --with-readline --with-openssl --with-zlib \
    --with-gd --with-jpeg-dir=/usr --with-libxml-dir=/usr
```

You'll need libvips 8.0 or later, including all the headers for
development.  On linux, install with your package manager.  On OS X,
install with `brew` or MacPorts. For Windows, download a zip from the
libvips website, or build your own.

### Installing

```
$ pear package
```

to make `vips-0.1.1.tgz`, then:

```
$ pear install vips-0.1.1.tgz
```

to install.

Add:

```
extension=vips.so
```

to your `php.ini`, perhaps in `~/vips/lib/php.ini`, if you configured php as
above. 

### Using

Try:

```php
#!/usr/bin/env php
<?php
if (!extension_loaded("vips")) {
    dl('vips.' . PHP_SHLIB_SUFFIX);
}

$x = vips_image_new_from_file($argv[1])["out"];
$x = vips_call("invert", $x)["out"];
vips_image_write_to_file($x, $argv[2]);
?>
```

And run with:

```
$ ./try1.php ~/pics/k2.jpg x.tif
```

See `examples/`.

### Development: regenerate build system

Run:

```
$ phpize
```

To scan `config.m4` and your php install and regenerate the build system.

### Development: configure

Run

```
$ ./configure 
```

Check the output carefully for errors, and obviously check that it found your
libvips.

### Development: make, test and install

Run:

```
$ make
```

To build the module to the `modules/` directory in this repository. 

Don't post php-vips test results to php.net! Stop this with:

```
$ export NO_INTERACTION=1
```

Test with:

```
$ make test
```

Finally, install to your php extensions area with:

```
$ make install
```

### Links

http://php.net/manual/en/internals2.php

https://devzone.zend.com/303/extension-writing-part-i-introduction-to-php-and-zend/

https://devzone.zend.com/317/extension-writing-part-ii-parameters-arrays-and-zvals/

https://devzone.zend.com/446/extension-writing-part-iii-resources/
