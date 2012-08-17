#ifndef _XPRINTF_H
#  define _XPRINTF_H

#define _GNU_SOURCE
#include <stdarg.h>
#include <stdlib.h>
#include <gcc-compat.h>

extern	unsigned		xprintf_set_debug( unsigned const );
extern	void _printf( 2, 3 )	xprintf( unsigned, const char *, ... );

#endif /* !_XPRINTF_H */
