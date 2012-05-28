#include <sys/types.h>
#include <vlm-tool.h>
#include <x-funcs.h>
#include <xprintf.h>

#include <stanzas.h>

/*
 *------------------------------------------------------------------------
 * Oops message highlighting
 *------------------------------------------------------------------------
 */

static	char *		oops_starters[] =	{
	"---[ cut here ]---",
	NULL
};

static	char *		oops_items[] =	{
	"---[ end trace .* ]---",
	NULL
};

static	stanza_t	oops_stanza =	{
	.name	  = "oops",
	.starters = oops_starters,
	.items	  = oops_items,
	.flags	  = STANZA_STOP,
	.budget   = 50
};

/*
 *------------------------------------------------------------------------
 * Kernel errors which show a stacktrace
 *------------------------------------------------------------------------
 */

static	char *		trace_starters[] =	{
	"unable to handle",
	"call trace:",
	"kobject_add failed for.*EXIST",
	NULL				/* Must be last			 */
};

static	char *		trace_items[] =	{
	"[[]<[0-9a-fA-F]*>[]](.*)?",
	NULL				/* Must be last			 */
};

static	stanza_t	trace_stanza =	{
	"trace",
	trace_starters,
	trace_items,
	0,
	50
};

/*
 *------------------------------------------------------------------------
 * Table of stanzas we will highlight
 *------------------------------------------------------------------------
 */

stanza_t *	stanzas[] = {
	&oops_stanza,
	&trace_stanza,
	NULL				/* Must be last			 */
};

static	void
stanza_setup_pool(
	char * *		list,	/* Null-terminated string list	 */
	pool_t * *		pp	/* Where pool pointer is kept	 */
)
{
	size_t			qty;

	/* Count length of string list					 */
	for( qty = 0; list[ qty ]; ++qty )	{
		/* NOTHING						 */
	}
	/* Allocate pool of exact length				 */
	*pp = pool_new( sizeof( trigger_t ), NULL, NULL );
	/* Fill in list of rules					 */
	for( qty = 0; ; ++qty )	{
		trigger_t * const	t = pool_alloc( *pp );

		xprintf( 2, "compiling pattern '%s'.", list[qty] );
		t->s = list[qty];
		if( regcomp(
			&(t->re),
			t->s,
			(REG_EXTENDED|REG_ICASE)
		) )	{
			perror( "out of pattern memory" );
			abort();
		}
	}
}

static	void
stanza_setup_one(
	stanza_t * const	stanza
)
{
	xprintf( 1, "processing starters for '%s'", stanza->name );
	stanza_setup_pool( stanza->starters, &(stanza->starter_pool) );
	xprintf( 1, "processing items for '%s'", stanza->name );
	stanza_setup_pool( stanza->items, &(stanza->item_pool) );
}

/*
 *------------------------------------------------------------------------
 * stanza_setup: prepare all known stanzas for use
 *------------------------------------------------------------------------
 */

void
stanza_setup(
	void
)
{
	stanza_t * *		stp;

	for( stp = stanzas; *stp; ++stp )	{
		xprintf( 1, "Compiling starters for '%s'", (*stp)->name );
		stanza_setup_one( (*stp) );
	}
}

/*
 *------------------------------------------------------------------------
 * stanza_find: locate stanza with matching rule, else NULL
 *------------------------------------------------------------------------
 */

stanza_t *
stanza_find(
	entry_t *		e	/* Candidate to match		 */
)
{
	stanza_t *		retval;

	retval = NULL;
	do	{
		stanza_t * *	stp;

		for( stp = stanzas; *stp; ++stp )	{
			pool_iter_t *	iter;
			trigger_t *	t;

			xprintf( 1, "examining '%s' for starters.", (*stp)->name );
			iter = pool_iter_new( (*stp)->starter_pool );
			do	{
				for(
					t = pool_iter_next( iter );
					t;
					t = pool_iter_next( iter )
				)	{
					regmatch_t	matches[10];

					if( !regexec(
						&t->re,
						e->resid,
						DIM(matches),
						matches,
						0
					) )	{
						e->trigger = t;
						retval = *stp;
						break;
					}
				}
			} while( 0 );
			pool_iter_free( &iter );
		}
	} while( 0 );
	return( retval );
}
