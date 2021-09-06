dnl $Id$
dnl config.m4 for extension vips

m4_include(m4/ax_require_defined.m4)
m4_include(m4/ax_append_flag.m4)
m4_include(m4/ax_check_link_flag.m4)
m4_include(m4/ax_append_link_flags.m4)

PHP_ARG_WITH(vips, for vips support,
[  --with-vips             Include vips support])

# 8.2 added vips_bandjoin_const(), which php-vips needs
VIPS_MIN_VERSION=8.2

if test x"$PHP_VIPS" != x"no"; then
  if ! pkg-config --atleast-pkgconfig-version 0.2; then
    AC_MSG_ERROR([you need at least pkg-config 0.2 for this module])
    PHP_VIPS=no
  fi
fi

if test x"$PHP_VIPS" != x"no"; then
  if ! pkg-config vips --atleast-version $VIPS_MIN_VERSION; then
    AC_MSG_ERROR([you need at least libvips $VIPS_MIN_VERSION for this module])
    PHP_VIPS=no
  fi
fi

if test x"$PHP_VIPS" != x"no"; then
  VIPS_CFLAGS=`pkg-config vips --cflags-only-other`
  VIPS_INCS=`pkg-config vips --cflags-only-I`
  VIPS_LIBS=`pkg-config vips --libs`

  PHP_CHECK_LIBRARY(vips, vips_init,
  [
    PHP_EVAL_INCLINE($VIPS_INCS)
    PHP_EVAL_LIBLINE($VIPS_LIBS, VIPS_SHARED_LIBADD)
  ],[
    AC_MSG_ERROR([libvips not found.  Check config.log for more information.])
  ],[$VIPS_LIBS]
  )

  # Mark DSO non-deletable at runtime.
  # See: https://github.com/libvips/php-vips-ext/issues/43
  AX_APPEND_LINK_FLAGS([-Wl,-z,nodelete])

  AC_DEFINE(HAVE_VIPS, 1, [Whether you have vips])
  PHP_NEW_EXTENSION(vips, vips.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1 $VIPS_CFLAGS)
  PHP_SUBST(VIPS_SHARED_LIBADD)
fi

