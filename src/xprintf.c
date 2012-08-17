#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <gcc-compat.h>

#include <xprintf.h>

static	FILE *		out;
static	unsigned	debug;

unsigned
xprintf_set_debug(
	unsigned const	level
)
{
	unsigned const	old = debug;

	debug = level;
	return( old );
}

void		_printf( 2, 3 )
xprintf(
	unsigned		level,	/* Print only if <= debug	 */
	const char * const	fmt,	/* Printf-style formatting	 */
	...				/* Arguments as required	 */
)
{
	if( out == NULL )	{
		out = stderr;
	}
	if( level <= debug )	{
		va_list		ap;
		char *		buff;

		va_start( ap, fmt );
		if( vasprintf( &buff, fmt, ap ) == -1 )	{
			buff = NULL;
		}
		va_end( ap );
		fprintf(
			out,
			"%s.\n",
			buff ? buff : "really out of memory"
		);
		free( buff );
	}
}
