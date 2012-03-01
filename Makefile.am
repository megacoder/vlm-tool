bin_PROGRAMS	= vlm-tool

vlm_tool_SOURCES = vlm-tool.c builtins.c builtins.h gcc-compat.h

AM_CFLAGS	=-I. `getconf LFS_CFLAGS` -DVERSION=\"@VERSION@\"
AM_LDFLAGS	=`getconf LFS_LDFLAGS`

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS README TODO autogen.sh \
	   configure.ac install-sh missing vlm-tool.1 vlm-tool.spec.in
