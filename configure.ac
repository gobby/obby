#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([obby], [0.4.8], [crew@0x539.de])
AM_INIT_AUTOMAKE(1.9 check-news)
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([inc/buffer.hpp])
AC_CONFIG_HEADER([inc/config.hpp])

# Extract host information.
AC_CANONICAL_HOST

# Checks for programs.
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_CACHE_SAVE

extra_includes=""
extra_libraries=""
extra_requires="sigc++-2.0 net6-1.3 >= 1.3.3"
AC_SUBST(extra_includes)
AC_SUBST(extra_libraries)
AC_SUBST(extra_requires)

# Initialise pkg-config.
PKG_CHECK_MODULES([libraries], [$extra_requires])

# IPv6 support
AC_ARG_ENABLE([ipv6],
              AS_HELP_STRING([--enable-ipv6],
	                     [enable IPv6 support]),
              [ipv6=$enableval], [ipv6=no])
AC_CACHE_CHECK([whether to enable IPv6 support],
               [ipv6], [ipv6=no])
if test "x$ipv6" = "xyes" ; then
  AC_DEFINE([USE_IPV6], 1, [Enable IPv6 support.])
fi

# Zeroconf support
AC_ARG_WITH([zeroconf],
            AS_HELP_STRING([--with-zeroconf],
                           [compile with Zeroconf support]),
            [zeroconf=$withval], [zeroconf=no])
AC_CACHE_CHECK([whether to compile with Zeroconf support],
               [zeroconf], [zeroconf=no])

using_zeroconf=no
if test "x$zeroconf" = "xyes" ; then
##  # Check for Apple's Bonjour first
##  AC_CHECK_HEADER(dns_sd.h, [AC_CHECK_LIB(dns_sd, DNSServiceRegister,
##                  [AC_DEFINE(WITH_BONJOUR, 1,
##                             [Use Bonjour Zeroconf implementation.])])])
##  if test "x$ac_cv_lib_dns_sd_DNSServiceRegister" = "xyes" ; then
##    using_zeroconf=bonjour
##    extra_libraries="$extra_libraries -ldns_sd"
##    AC_MSG_NOTICE([Using Bonjour/DNS-SD Zeroconf implementation.])
##  else
    PKG_CHECK_MODULES([libavahi], [avahi-client >= 0.6], [avahi_found=yes],
                      [avahi_found=no])
    if test "x$avahi_found" = "xyes" ; then
      using_zeroconf=avahi
      extra_requires="$extra_requires avahi-client"
      AC_DEFINE(WITH_AVAHI, 1, [Use Avahi Zeroconf implementation.])
      AC_MSG_NOTICE([Using Avahi Zeroconf implementation.])
    else
      PKG_CHECK_MODULES([libhowl], [howl], [howl_found=yes], [howl_found=no])
      if test "x$howl_found" = "xyes" ; then
        using_zeroconf=howl
        extra_requires="$extra_requires howl"
        AC_DEFINE(WITH_HOWL, 1, [Use Howl Zeroconf implementation.])
        AC_MSG_NOTICE([Using Howl Zeroconf implementation.])
      else
        AC_MSG_ERROR([No supported Zeroconf implementation found.])
        exit
      fi
    fi
##  fi
fi
AM_CONDITIONAL(WITH_ZEROCONF, test x$using_zeroconf != xno)
AM_CONDITIONAL(WITH_BONJOUR, test x$using_zeroconf = xbonjour)
AM_CONDITIONAL(WITH_HOWL, test x$using_zeroconf = xhowl)
AM_CONDITIONAL(WITH_AVAHI, test x$using_zeroconf = xavahi)
AC_CACHE_SAVE

# WIN32 build checks
AC_MSG_CHECKING([whether to enable WIN32 specific flags])
case "$host_os" in
*mingw*)
	win32=true
	AC_MSG_RESULT([yes])
	;;
*)
	win32=false
	AC_MSG_RESULT([no])
	;;
esac
AM_CONDITIONAL(WIN32, test x$win32 = xtrue)

# Checks for libraries.
PKG_CHECK_MODULES([libobby], [$extra_requires])

# gettext / i18n
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.15])

AC_CONFIG_FILES([Makefile po/Makefile.in inc/Makefile src/Makefile
                 src/serialise/Makefile test/Makefile obby-0.4.pc])
AC_OUTPUT

