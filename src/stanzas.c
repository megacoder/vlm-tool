#include <sys/types.h>
#include <vlm-tool.h>
#include <x-funcs.h>
#include <xprintf.h>
#include <utils.h>

#include <stanzas.h>

/*
 *------------------------------------------------------------------------
 * Oops message highlighting
 *------------------------------------------------------------------------
 */

static	char *		oops_starters[] =	{
	"---\\[ cut here \\]---",
	NULL
};

static	char *		oops_items[] =	{
	"---\\[ end trace .* \\]---",
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
	.name	  = "trace",
	.starters = trace_starters,
	.items	  = trace_items,
	.flags	  = 0,
	.budget   = 50
};

/*
 *------------------------------------------------------------------------
 * Task-related messages
 *------------------------------------------------------------------------
 */

static	char *		task_starters[] =	{
	"INFO: task.*blocked for more than",
	NULL				/* Must be last			 */
};

static	char *		task_items[] =	{
	".*",				/* Highlight the entire line	 */
	NULL				/* Must be last			 */
};

static	stanza_t	task_stanza =	{
	.name	  = "task",
	.starters = task_starters,
	.items	  = task_items,
	.flags	  = 0,
	.budget   = 2
};

/*
 *------------------------------------------------------------------------
 * PVM-related messages
 *------------------------------------------------------------------------
 */

static	char *		pvm_starters[] =	{
	"You might have to change the root device.*",
	NULL				/* Must be last			 */
};

static	char *		pvm_items[] =	{
	".*",				/* Highlight the entire line	 */
	NULL				/* Must be last			 */
};

static	stanza_t	pvm_stanza =	{
	.name	  = "pvm",
	.starters = pvm_starters,
	.items	  = pvm_items,
	.flags	  = 0,
	.budget   = 3			/* No ender; programmer's trick	 */
};

/*
 *------------------------------------------------------------------------
 * Table of stanzas we will highlight
 *------------------------------------------------------------------------
 */

stanza_t *	stanzas[] = {
	&oops_stanza,
	&trace_stanza,
	&task_stanza,
	&pvm_stanza,
	NULL				/* Must be last			 */
};

static	void
stanza_setup_pool(
	char * *		list,	/* Null-terminated string list	 */
	pool_t * *		pp	/* Where pool pointer is kept	 */
)
{
	size_t			qty;
	size_t			i;

	/* Count length of string list					 */
	for( qty = 0; list[ qty ]; ++qty )	{
		/* NOTHING						 */
	}
	/* Allocate pool of exact length				 */
	*pp = pool_new( sizeof( trigger_t ), NULL, NULL );
	/* Fill in list of rules					 */
	for( i = 0; i < qty; ++i )	{
		trigger_t * const	t = pool_alloc( *pp );

		trigger_compile( list[i], t );
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
 * stanza_search_one: scan stanza for matching rule, else NULL
 *------------------------------------------------------------------------
 */

stanza_t *
stanza_search_one(
	stanza_t *		stanza,	/* Where to search		 */
	entry_t *		e,	/* Candidate to match		 */
	int const		begin	/* Search starters if true	 */
)
{
	stanza_t *		retval;

	retval = NULL;
	do	{
		pool_t * const	pool =
			begin ? stanza->starter_pool : stanza->item_pool;
		pool_iter_t *	iter;
		trigger_t *	t;

		iter = pool_iter_new( pool );
		for(
			t = pool_iter_next( iter );
			t;
			t = pool_iter_next( iter )
		)	{
			if(trigger_match( e->resid, t, NULL )){
				xprintf(
					2,
					"matched '%s' by '%s'.",
					t->s,
					e->resid
				);
				e->trigger = t;
				retval = stanza;
				break;
			}
		}
		pool_iter_free( &iter );
	} while( 0 );
	return( retval );
}

/*
 *------------------------------------------------------------------------
 * stanza_search_all: locate stanza with matching rule, else NULL
 *------------------------------------------------------------------------
 */

stanza_t *
stanza_search_starters(
	entry_t *		e	/* Candidate to match		 */
)
{
	stanza_t *		retval;

	retval = NULL;
	do	{
		stanza_t *	s;
		stanza_t * *	stp;

		for( stp = stanzas; *stp; ++stp )	{
			s = stanza_search_one( *stp, e, 1 );
			if( s )	{
				retval = s;
				break;
			}
		}
	} while( 0 );
	return( retval );
}
