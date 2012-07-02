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
	stanza_t *			host_states[ hosts_qty ];
	uint16_t			stanza_budget[ hosts_qty ];
	trigger_t *			stanza_triggers[ hosts_qty ];

	xprintf( 1, "Postprocessing" );
	/* We ain't got nuthin' yet					 */
	stanza_setup();
	/* Host states: NULL=looking, else=ender table			 */
	memset( host_states, 0, sizeof( host_states[0] ) );
	memset( stanza_budget, 0, sizeof( stanza_budget[0] ) );
	memset( stanza_triggers, 0, sizeof( stanza_triggers[0] ) );
	/* Iterate over all the entries, looking for a starter		 */
	xprintf( 1, "applying starters" );
	iter = pool_iter_new( entries );
	for(
		e = pool_iter_next( iter );
		e;
		e = pool_iter_next( iter )
	)	{
		size_t const	host_id = e->host_id;

		/* Called once for each /v/l/m entry we've kept		 */
		if( !host_states[host_id] )	{
			/* Haven't found stanza yet, maybe this one	 */
			host_states[host_id] = stanza_find( e, 1 );
			if( host_states[host_id] )	{
				/* Begins stanza, establish budget	 */
				stanza_triggers[host_id] = e->trigger;
				stanza_budget[host_id] =
					host_states[host_id]->budget;
			}
		} else	{
			int	done;

			/* In a stanza, see if we match any item rule	 */
			done = 0;
			/* First, test the stanza's budget		 */
			if(
				(stanza_budget[host_id] > 0) &&
				(--stanza_budget[host_id] == 0)
			)	{
				done                     = 1;
				/* Budget expired			 */
				host_states[host_id]     = NULL;
				stanza_triggers[host_id] = NULL;
			} else	{
				/* Under budget				 */
				int const	match_stops = (
					host_states[host_id]->flags &
					STANZA_STOP
				);

				if( stanza_find( e, 0 ) )	{
					/* Found an item match		 */
					e->trigger = stanza_triggers[host_id];
					if( match_stops )	{
						done = 1;
					}
				} else	{
					/* Didn't find an item match	 */
					if( match_stops )	{
						/* Keep going		 */
						e->trigger =
							stanza_triggers[host_id];
					} else	{
						/* Must match to be in	 */
						done = 1;
					}
				}
				if( done )	{
					host_states[host_id]        = NULL;
					stanza_triggers[e->host_id] = NULL;
				}
			}
		}
	}
}
