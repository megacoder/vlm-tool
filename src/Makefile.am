bin_PROGRAMS	= vlm-tool
man_MANS	= vlm-tool.1

vlm_tool_SOURCES=builtins.c builtins.h gcc-compat.h pool.c pool.h      \
         post-process.c post-process.h stanzas.c stanzas.h utils.c     \
         utils.h vlm-tool.c vlm-tool.h x-funcs.h xprintf.c xprintf.h

AM_CFLAGS	=-std=gnu99 `getconf LFS_CFLAGS` -DVERSION=\"@VERSION@\" -I. -Wall -Werror -pedantic -g -Wsuggest-attribute=pure
AM_LDFLAGS	=`getconf LFS_LDFLAGS` -g -lm

EXTRA_DIST=vlm-tool.1
