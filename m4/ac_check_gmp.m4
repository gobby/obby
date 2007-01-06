dnl A function to check for the existence and usability of GMP.
dnl Copyright (C) 2001-2005 Roberto Bagnara <bagnara@cs.unipr.it>
dnl
dnl This file was part of the Parma Polyhedra Library (PPL) and is now
dnl part of the obby library. Both obby and the PPL are redistributed
dnl under the GNU General Public License as follows:
dnl
dnl This library is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU General Public License as published by the
dnl Free Software Foundation; either version 2 of the License, or (at your
dnl option) any later version.
dnl
dnl The PPL is distributed in the hope that it will be useful, but WITHOUT
dnl ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
dnl FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
dnl for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
dnl USA.
dnl
AC_DEFUN([AC_CHECK_GMP],
[
dnl Since libgmp and libgmpxx are usually installed in the same location,
dnl let the prefixes default from each other.
if test -n "$with_libgmpxx_prefix" && test -z "$with_libgmp_prefix"; then
  with_libgmp_prefix="$with_libgmpxx_prefix"
else
  if test -n "$with_libgmp_prefix" && test -z "$with_libgmpxx_prefix"; then
    with_libgmpxx_prefix="$with_libgmp_prefix"
  fi
fi

dnl Check how to link with libgmp.
AC_LIB_LINKFLAGS([gmp])

dnl Check how to link with libgmpxx.
AC_LIB_LINKFLAGS([gmpxx], [gmp])

ac_save_LIBS="$LIBS"
LIBS="$LIBS $LIBGMPXX"
AC_LANG_PUSH(C++)

AC_MSG_CHECKING([for the GMP library version 4.1.3 or above])
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <gmpxx.h>

#if __GNU_MP_VERSION < 4 || (__GNU_MP_VERSION == 4 && __GNU_MP_VERSION_MINOR < 1) || (__GNU_MP_VERSION == 4 && __GNU_MP_VERSION_MINOR == 1 && __GNU_MP_VERSION_PATCHLEVEL < 3)
#error "GMP version 4.1.3 or higher is required"
#endif

int main() {
  mpz_class n("3141592653589793238462643383279502884");
  return 0;
}
]])],
  AC_MSG_RESULT(yes)
  ac_cv_have_gmp=yes,
  AC_MSG_RESULT(no)
  ac_cv_have_gmp=no,
  AC_MSG_RESULT(no)
  ac_cv_have_gmp=no)

have_gmp=${ac_cv_have_gmp}

if test x"$ac_cv_have_gmp" = xyes
then

AC_CHECK_SIZEOF(mp_limb_t, , [#include <gmp.h>])

AC_MSG_CHECKING([whether GMP has been compiled with support for exceptions])
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <gmpxx.h>
#include <new>
#include <cstddef>

static void*
x_malloc(size_t) {
  throw std::bad_alloc();
}

static void*
x_realloc(void*, size_t, size_t) {
  throw std::bad_alloc();
}

static void
x_free(void*, size_t) {
}

int main() {
  mp_set_memory_functions(x_malloc, x_realloc, x_free);
  try {
    mpz_class n("3141592653589793238462643383279502884");
  }
  catch (std::bad_alloc&) {
    return 0;
  }
  return 1;
}
]])],
  AC_MSG_RESULT(yes)
  ac_cv_gmp_supports_exceptions=yes,
  AC_MSG_RESULT(no)
  ac_cv_gmp_supports_exceptions=no,
  AC_MSG_RESULT(no)
  ac_cv_gmp_supports_exceptions=no)

gmp_supports_exceptions=${ac_cv_gmp_supports_exceptions}
if test x"$gmp_supports_exceptions" = xyes
then
  value=1
else
  value=0
fi
AC_DEFINE_UNQUOTED(GMP_SUPPORTS_EXCEPTIONS, $value,
  [Not zero if GMP has been compiled with support for exceptions.])

fi

AC_LANG_POP(C++)
LIBS="$ac_save_LIBS"

dnl We use libtool, therefore we take $LTLIBGMPXX, not $LIBGMPXX.
gmp_library_option="$LTLIBGMPXX"
])
