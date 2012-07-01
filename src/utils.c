#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#include <vlm-tool.h>
#include <xprintf.h>
#include <stanzas.h>

#include <utils.h>

/*
 *------------------------------------------------------------------------
 * trigger_compile: compile string into a trigger pattern
 *------------------------------------------------------------------------
 */

void
trigger_compile(
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

/*
 *------------------------------------------------------------------------
 * trigger_match: regular expression processing
 *------------------------------------------------------------------------
 * There may be up to NUM_MATCHES partial matches returned. If you are not
 * interested in them, you can just pass a NULL pointer in the 'matches'
 * parameter.
 *------------------------------------------------------------------------
 */

int
trigger_match(
	char const * const	s,	/* String to be considered	 */
	trigger_t * const	t,	/* Trigger to be tested		 */
	regmatch_t *		matches	/* Where to store segments	 */
)
{
	int		retval;
	regmatch_t	dummies[ NUM_MATCHES ];

	/* Invert sense of return value: 0 is no match, 1 is a match	 */
	retval = !regexec(
		&(t->re),		/* Compiled regex address	 */
		s,			/* Candidate string address	 */
		NUM_MATCHES,		/* Support at most this many	 */
		matches ? matches: dummies,/* Keep or toss		 */
		0
	);
	return( retval );
}
