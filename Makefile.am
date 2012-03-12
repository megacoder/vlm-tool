bin_PROGRAMS	= vlm-tool
man_MANS	= vlm-tool.1

vlm_tool_SOURCES = vlm-tool.c builtins.c builtins.h gcc-compat.h pool.h pool.c \
		   xprintf.h

AM_CFLAGS	=-std=gnu99 `getconf LFS_CFLAGS` -DVERSION=\"@VERSION@\" -I. -g -O0 -Wall -Werror -pedantic
AM_LDFLAGS	=`getconf LFS_LDFLAGS` -g

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS README TODO autogen.sh \
	   configure.ac install-sh missing vlm-tool.1 vlm-tool.spec.in

