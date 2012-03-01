#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

VERSION=v1.0.0
AC_PREREQ([2.66])
AC_INIT([vlm_tool], VERSION, [Tommy.Reynolds@MegaCoder.com])
AC_CONFIG_SRCDIR([gcc-compat.h])
AC_CONFIG_HEADERS([config.h])

AC_SUBST(VERSION)

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h malloc.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset regcomp strdup strrchr strtol strtoul])

AC_CONFIG_FILES([
	 Makefile
	 vlm_tool.spec
])
AC_OUTPUT
