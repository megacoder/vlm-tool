bin_PROGRAMS	= vlm-tool
man_MANS	= vlm-tool.1

vlm_tool_SOURCES = vlm-tool.c builtins.c builtins.h gcc-compat.h

AM_CFLAGS	=`getconf LFS_CFLAGS` -DVERSION=\"@VERSION@\" -I.
AM_LDFLAGS	=`getconf LFS_LDFLAGS`

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS README TODO autogen.sh \
	   configure.ac install-sh missing vlm-tool.1 vlm-tool.spec.in
