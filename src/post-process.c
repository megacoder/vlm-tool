#include <sys/types.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pool.h>
#include <xprintf.h>
#include <x-funcs.h>
#include <stanzas.h>
#include <vlm-tool.h>

#include <post-process.h>

void
post_process(
	void
)
{
	entry_t *			e;
	pool_iter_t *			iter;
	stanza_t *			in_stanza[ hosts_qty ];
	uint16_t			stanza_budget[ hosts_qty ];
	trigger_t *			began_with[ hosts_qty ];

	xprintf( 1, "Postprocessing" );
	/* We ain't got nuthin' yet					 */
	stanza_setup();
	/* Host states: NULL=looking, else=ender table			 */
	memset( in_stanza,     0, sizeof( in_stanza     ) );
	memset( stanza_budget, 0, sizeof( stanza_budget ) );
	memset( began_with,    0, sizeof( began_with    ) );
	/* Iterate over all the entries, looking for a starter		 */
	xprintf( 1, "applying starters" );
	iter = pool_iter_new( entries );
	for(
		e = pool_iter_next( iter );
		e;
		e = pool_iter_next( iter )
	)	{
		/* Called once for each /v/l/m entry we've kept		 */
		size_t const	hid = e->host_id;

		/* First, check for ender matching			 */
		if( in_stanza[hid] )	{
			int const	match_ends_stanza = (
				in_stanza[hid]->flags & STANZA_STOP
			);
			int const	mark_ender =	(
				in_stanza[hid]->flags & STANZA_ISLAST
			);
			/* In a stanza, try to locate end		 */
			int		stanza_ended;
			stanza_t *	matched_stanza;

			e->trigger     = began_with[hid];
			stanza_ended   = 0;
			matched_stanza = NULL;
			/* Check remaining budget			 */
			if(
				(stanza_budget[hid] > 0) &&
				( --stanza_budget[hid] == 0 )
			)	{
				/* Budget expired			 */
				stanza_ended		= 1;
			} else	{
				/* Under budget				 */
				matched_stanza = stanza_search_one(
					in_stanza[hid],
					e,
					0
				);
				if( matched_stanza && match_ends_stanza ) {
					stanza_ended = 1;
				}
				if( !matched_stanza && !match_ends_stanza ) {
					stanza_ended = 1;
				}
			}
			if( !stanza_ended )	{
				continue;
			}
			/* Stanza ends			 */
			if( !mark_ender )	{
				e->trigger	   = NULL;
			}
			stanza_budget[hid] = 0;
			in_stanza[hid]	   = NULL;
			began_with[hid]    = NULL;
			/*
			 * Fall through and consider if this
			 * entry starts a new stanza.
			 */
		}
		/* Current entry is candidate to begin a stanza		 */
		in_stanza[hid] = stanza_search_starters( e );
		if( in_stanza[hid] )	{
			/* New stanza has begun */
			began_with[hid] = e->trigger;
			stanza_budget[hid] = in_stanza[hid]->budget;
		}
	}
}
