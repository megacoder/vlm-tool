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
	memset( in_stanza, 0, hosts_qty * sizeof( in_stanza[0] ) );
	memset( stanza_budget, 0, hosts_qty * sizeof( stanza_budget[0] ) );
	memset( began_with, 0, hosts_qty * sizeof( began_with[0] ) );
	/* Iterate over all the entries, looking for a starter		 */
	xprintf( 1, "applying starters" );
	iter = pool_iter_new( entries );
	for(
		e = pool_iter_next( iter );
		e;
		e = pool_iter_next( iter )
	)	{
		size_t const	hid = e->host_id;

		/* Called once for each /v/l/m entry we've kept		 */
		if( !in_stanza[hid] )	{
			/* Haven't found stanza yet, maybe this one	 */
			in_stanza[hid] = stanza_search_starters( e );
			if( in_stanza[hid] )	{
				/* Begins stanza, establish budget	 */
				began_with[hid] = e->trigger;
				stanza_budget[hid] = in_stanza[hid]->budget;
			}
		} else	{
			int	done;

			/* In a stanza, see if we match any item rule	 */
			done = 0;
			/* First, test the stanza's budget		 */
			if(
				(stanza_budget[hid] > 0) &&
				(--stanza_budget[hid] == 0)
			)	{
				/* Budget expired			 */
				done            = 1;
			} else	{
				/* Under budget				 */
				int const	match_stops = (
					in_stanza[hid]->flags &
					STANZA_STOP
				);
				stanza_t *	s;

				s = stanza_search_one( in_stanza[hid], e, 0 );
				if( s )	{
					/* Found an item match		 */
					e->trigger = began_with[hid];
					if( match_stops )	{
						done = 1;
					}
				} else	{
					/* Didn't find an item match	 */
					if( match_stops )	{
						/* Keep going		 */
						e->trigger =
							began_with[hid];
					} else	{
						/* Must match to be in	 */
						e->trigger = NULL;
						done = 1;
					}
				}
			}
			if( done )	{
				/* This entry ended stanza		 */
				stanza_budget[hid] = 0;
				in_stanza[hid]  = NULL;
				began_with[hid] = NULL;
			}
		}
	}
}
