#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#

AC_PREREQ(2.61)
AC_INIT([tdu], 0.4, tbrown@freeshell.org)
AC_CONFIG_AUX_DIR([build-aux])

# Create the VERSION file, which contains the package version from
# AC_INIT.
echo AC_PACKAGE_VERSION>VERSION
AC_SUBST(PACKAGE_VERSION)

# Internationalization
AM_GNU_GETTEXT_VERSION([0.18.1])
AM_GNU_GETTEXT([external])

# Initialise automake
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# define our M4 macro directory
AC_CONFIG_MACRO_DIR([m4])

# Check there is a source file
AC_CONFIG_SRCDIR([src/main.c])

# Put our generated config header in the source directory
AC_CONFIG_HEADERS([src/config.h])

# Checks the compiler vendor
AX_COMPILER_VENDOR
AC_PROG_CC
AC_PROG_CC_STDC

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([assert.h err.h ftw.h getopt.h libgen.h limits.h \
                  locale.h poll.h search.h stdint.h stdio.h       \
                  stdlib.h string.h sys/resource.h sys/time.h     \
                  sys/types.h sysexits.h time.h unistd.h])
AC_CHECK_FUNCS([memset getprogname program_invocation_short_name twalk])

dnl override CFLAGS selection when debugging
AC_ARG_ENABLE([debug],
        AC_HELP_STRING([--enable-debug], [compile for debugging]),
        [enable_debug=$enableval],
        [enable_debug=no])
if test "x$enable_debug" = "xyes"; then
        AC_DEFINE([DEBUG], [1], [Enable debugging code])
fi

AC_CONFIG_FILES([Makefile
                 po/Makefile.in
                 src/Makefile
                 ])

AC_OUTPUT

