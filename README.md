# Low-level PHP binding for libvips 

This extension lets you use the libvips image processing library from PHP 7. It 
is intentionally very low-level: modules such as 
https://github.com/jcupitt/php-vips try to layer a nice API on top of this.

libvips is fast and needs little memory. The [`vips-php-bench`](
https://github.com/jcupitt/php-vips-bench) repository tests
`php-vips` against `imagick` and `gd`: on that test, and on my laptop,
`php-vips` is around four times faster than `imagick` and needs 10 times less
memory. 

### Example

```php
#!/usr/bin/env php
<?php

$x = vips_image_new_from_file($argv[1])["out"];
$x = vips_call("invert", $x)["out"];
vips_image_write_to_file($x, $argv[2]);
```

Almost all operations return an array of result values. Usually there is a
single result called `"out"`.

Use `vips_call()` to call any operation in the vips library. There are around 
around 300 operations available, see the vips docs for an introduction:

http://jcupitt.github.io/libvips/API/current/

Arguments can be long, double, image, array of long, array of double or array
of image. The final argument to `vips_call()` is an array of operation options. 

`php-vips` layers a nice API, including full docs, on top of this extension, 
see:

https://github.com/jcupitt/php-vips

### Installing

First install the libvips library. It will be in your package manager on linux,
it's in brew and MacPorts on macOS, or the vips website has Windows binaries.

Next, install this extension:

```
$ pecl install vips
```

And add:

```
extension=vips.so
```

to your `php.ini`.

Finally, add `vips` to your `composer.json` to pull in the high-level PHP API. 

```
    "require": {
            "jcupitt/vips" : "1.0.0"
    }
```

The high-level API has all the documentation, see:

https://github.com/jcupitt/php-vips

### Development: preparation

PHP is normally built for speed and is missing a lot of debugging support you
need for extension development. For testing and dev, build your own php. 
I used 7.0.11 and configured with:

```
$ ./configure --prefix=/home/john/vips --enable-debug --enable-maintainer-zts \
    --enable-cgi --enable-cli --with-readline --with-openssl --with-zlib \
    --with-gd --with-jpeg-dir=/usr --with-libxml-dir=/usr --enable-mbstring 
```

You'll need libvips 8.2 or later, including all the headers for
development.  On linux, install with your package manager.  On macOS,
install with `brew` or MacPorts. For Windows, download a zip from the
libvips website, or build your own.

### Development: regenerate build system

```
$ pear package
```

to make `vips-1.0.7.tgz`.

To install by hand:

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

Add `extension=vips.so` to `php.ini`, perhaps in `~/vips/lib/php.ini`, 
if you configured php as above. 

### Links

http://php.net/manual/en/internals2.php

https://devzone.zend.com/303/extension-writing-part-i-introduction-to-php-and-zend/

https://devzone.zend.com/317/extension-writing-part-ii-parameters-arrays-and-zvals/

https://devzone.zend.com/446/extension-writing-part-iii-resources/
