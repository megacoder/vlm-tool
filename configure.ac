#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.4])
AC_INIT([vlm-tool], [2.1.1], [Tommy.Reynolds@MegaCoder.com])
AC_CONFIG_SRCDIR([src/gcc-compat.h])
AC_CONFIG_HEADERS([config.h])

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
	 src/Makefile
	 src/vlm-tool.1
	 vlm-tool.spec
])
AC_OUTPUT
