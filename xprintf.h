#ifndef _XPRINTF_H
#  define _XPRINTF_H

#if	1

#  include <stdio.h>
#  define xprintf(l,a)	  if( (l) <= debug ) {printf a;}

#else	/* NOPE */

#include <stdarg.h>
#include <gcc-compat.h>

static	void		_printf( 2, 3 )
xprintf(
	unsigned		level,	/* Print only if <= debug	 */
	const char * const	fmt,	/* Printf-style formatting	 */
	...				/* Arguments as required	 */
)
{
	if( level <= debug )	{
		va_list		ap;

		va_start( ap, fmt );
		vprintf( fmt, ap );
		va_end( ap );
	}
}
#  endif /* NOPE */
#endif /* !_XPRINTF_H */
