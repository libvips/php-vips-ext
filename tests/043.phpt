--TEST--
can get fields
--SKIPIF--
<?php if (!extension_loaded("vips")) print "skip"; ?>
--FILE--
<?php
  $filename = dirname(__FILE__) . "/images/img_0076.jpg";
  $image = vips_image_new_from_file($filename)["out"];

  $result = vips_image_get_fields($image);

  if ($result === -1) {
     echo("Result is -1\n");
  }

  $fields = $result["out"];

  foreach($fields as $field) {
     echo("pass $field\n");
  }

  echo("pass get_fields\n");
?>
--EXPECT--
pass width
pass height
pass bands
pass format
pass coding
pass interpretation
pass xoffset
pass yoffset
pass xres
pass yres
pass filename
pass vips-loader
pass jpeg-multiscan
pass jpeg-chroma-subsample
pass exif-data
pass resolution-unit
pass exif-ifd0-Make
pass exif-ifd0-Model
pass exif-ifd0-Orientation
pass exif-ifd0-XResolution
pass exif-ifd0-YResolution
pass exif-ifd0-ResolutionUnit
pass exif-ifd0-DateTime
pass exif-ifd0-YCbCrPositioning
pass exif-ifd1-Compression
pass exif-ifd1-XResolution
pass exif-ifd1-YResolution
pass exif-ifd1-ResolutionUnit
pass exif-ifd2-ExposureTime
pass exif-ifd2-FNumber
pass exif-ifd2-ExifVersion
pass exif-ifd2-DateTimeOriginal
pass exif-ifd2-DateTimeDigitized
pass exif-ifd2-ComponentsConfiguration
pass exif-ifd2-CompressedBitsPerPixel
pass exif-ifd2-ShutterSpeedValue
pass exif-ifd2-ApertureValue
pass exif-ifd2-ExposureBiasValue
pass exif-ifd2-MaxApertureValue
pass exif-ifd2-SubjectDistance
pass exif-ifd2-MeteringMode
pass exif-ifd2-Flash
pass exif-ifd2-FocalLength
pass exif-ifd2-MakerNote
pass exif-ifd2-UserComment
pass exif-ifd2-FlashPixVersion
pass exif-ifd2-ColorSpace
pass exif-ifd2-PixelXDimension
pass exif-ifd2-PixelYDimension
pass exif-ifd2-FocalPlaneXResolution
pass exif-ifd2-FocalPlaneYResolution
pass exif-ifd2-FocalPlaneResolutionUnit
pass exif-ifd2-SensingMethod
pass exif-ifd2-FileSource
pass exif-ifd4-InteroperabilityIndex
pass exif-ifd4-InteroperabilityVersion
pass exif-ifd4-RelatedImageWidth
pass exif-ifd4-RelatedImageLength
pass jpeg-thumbnail-data
pass orientation
pass get_fields

