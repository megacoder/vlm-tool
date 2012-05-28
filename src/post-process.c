#include <sys/types.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gcc-compat.h>
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
	stanza_t *			host_states[ hosts_qty ];
	uint16_t			stanza_budget[ hosts_qty ];

	xprintf( 1, "Postprocessing" );
	/* We ain't got nuthin' yet					 */
	stanza_setup();
	/* Host states: NULL=looking, else=ender table			 */
	memset( host_states, 0, sizeof( host_states[0] ) );
	memset( stanza_budget, 0, sizeof( stanza_budget[0] ) );
	/* Iterate over all the entries, looking for a starter		 */
	xprintf( 1, "applying starters" );
	iter = pool_iter_new( entries );
	for(
		e = pool_iter_next( iter );
		e;
		e = pool_iter_next( iter )
	)	{
		/* Called once for each /v/l/m entry we've kept		 */
		if( !host_states[e->host_id] )	{
			/* Haven't found stanza yet, maybe this one	 */
			host_states[e->host_id] = stanza_find( e );
			if( host_states[e->host_id] )	{
				/* Begins stanza, establish budget	 */
				stanza_budget[e->host_id] =
					host_states[e->host_id]->budget;
			}
		} else	{
			/* In a stanza, see if we match any item rule	 */
			/* First, test the stanza's budget		 */
			if(
				(stanza_budget[e->host_id] > 0) &&
				(--stanza_budget[e->host_id] == 0)
			)	{
				/* Budget expired			 */
				host_states[e->host_id] = NULL;
			} else	{
				/* Under budget				 */
				int const	match_stops = (
					host_states[e->host_id]->flags &
					STANZA_STOP
				);
				stanza_t *	match;

				match = stanza_find( e );
				if( match_stops && (match != NULL) )	{
					host_states[e->host_id] = NULL;
				} else if( !match_stops && (match == NULL) ) {
					host_states[e->host_id] = NULL;
				}
			}
		}
	}
}
