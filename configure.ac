#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([obby], [0.4.0rc1], [crew@0x539.de])
AM_INIT_AUTOMAKE(1.9)
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([inc/buffer.hpp])
AC_CONFIG_HEADER([inc/config.hpp])

# Extract host information.
AC_CANONICAL_HOST

# Checks for programs.
AC_LANG(C++)
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_CACHE_SAVE

extra_includes=""
extra_libraries=""
extra_requires="sigc++-2.0 net6-1.3"
AC_SUBST(extra_includes)
AC_SUBST(extra_libraries)
AC_SUBST(extra_requires)

# Zeroconf support
zeroconf=no

## Let the user choose if he wants libhowl support or not (default to no).
AC_ARG_WITH([howl],
	    AS_HELP_STRING([--with-howl],
			   [support Zeroconf through libhowl [[default=no]]]),
	    [ac_cv_use_howl=$withval], [ac_cv_use_howl=no])
AC_CACHE_CHECK([whether to use howl], [ac_cv_use_howl], [ac_cv_use_howl=no])
if test "x$ac_cv_use_howl" != "xno" ; then
	extra_requires="$extra_requires howl"
	zeroconf=yes
	AC_DEFINE([WITH_HOWL], 1, [Define whether to use howl.])
fi

## Let the user choose if he wants to support Zeroconf through Avahi
## (default to no).
AC_ARG_WITH([avahi],
            AS_HELP_STRING([--with-avahi],
                           [support Zeroconf through Avahi [[default=no]]]),
            [ac_cv_use_avahi=$withval], [ac_cv_use_howl=np])
AC_CACHE_CHECK([whether to use Avahi], [ac_cv_use_avahi], [ac_cv_use_avahi=no])
if test "x$ac_cv_use_avahi" != "xno" ; then
        extra_requires="$extra_requires avahi-client"
	zeroconf=yes
	AC_DEFINE([WITH_AVAHI], 1, [Define whether to use Avahi.])
fi

if test "x$ac_cv_use_howl" != "xno" ; then
  if test "x$ac_cv_use_avahi" != "xno" ; then
    AC_MSG_ERROR([[
*** You cannot use both libhowl and Avahi. Try to use only one of them.
]])
  fi
fi

AM_CONDITIONAL(WITH_ZEROCONF, test x$zeroconf = xyes)

# WIN32 build checks
AC_MSG_CHECKING([whether to enable WIN32 specific flags])
if test "x$host_os" = "xmingw32" ; then
        win32=true
        AC_MSG_RESULT([yes])
else
        win32=false
        AC_MSG_RESULT([no])
fi
AM_CONDITIONAL(WIN32, test x$win32 = xtrue)

# Allow to disable the build of the tests.
AC_ARG_ENABLE([tests],
	      AS_HELP_STRING([--enable-tests],
			     [enable the build of the tests]),
	      [ac_cv_build_tests=$enableval], [ac_cv_build_tests=no])
AC_CACHE_CHECK([whether to build the tests], [ac_cv_build_tests],
	       [ac_cv_build_tests=no])
AM_CONDITIONAL(BUILD_TESTS, test x$ac_cv_build_tests != xno)

# Checks for libraries.
PKG_CHECK_MODULES([libobby], [$extra_requires])
if test "x$ac_cv_use_howl" != "xno" ; then
	PKG_CHECK_MODULES([libhowl], [howl])
fi

# gettext / i18n
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION

AC_CONFIG_FILES([Makefile po/Makefile.in inc/Makefile src/Makefile
                 src/serialise/Makefile test/Makefile obby-0.4.pc])
AC_OUTPUT

