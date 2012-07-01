#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#include <vlm-tool.h>
#include <xprintf.h>
#include <stanzas.h>

#include <utils.h>

/*
 *------------------------------------------------------------------------
 * compile_trigger: compile string into a trigger pattern
 *------------------------------------------------------------------------
 */

void
compile_trigger(
	char const * const	s,	/* Regular expression string	 */
	trigger_t * const	t	/* Compile into this trigger	 */
)
{
	xprintf( 2, "compiling pattern '%s'.", s );
	t->s = s;
	if( regcomp(
		&(t->re),
		t->s,
		(REG_EXTENDED|REG_ICASE)
	) )	{
		perror( "out of pattern memory" );
		abort();
	}
}
