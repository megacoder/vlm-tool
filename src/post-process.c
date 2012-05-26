#include <sys/types.h>
#include <regex.h>
#include <gcc-compat.h>
#include <pool.h>
#include <xprintf.h>

#include <vlm-tool.h>
#include <post-process.h>

void
post_process(
	void
)
{
	static char const * const	start_strings[] =	{
		"unable to handle",
		"call trace:",
		"kobject_add failed for.*EXIST",
		NULL
	};
	static char const * const	end_strings[] =	{
		"[[]<[0-9a-fA-F]*>[]](.*)?",
		NULL
	};
	char const * const *		s;
	pool_t *			starters;
	pool_t *			enders;
	entry_t *			e;
	pool_iter_t *			iter;
	unsigned char *			host_states;

	xprintf( 1, "Postprocessing" );
	/* We ain't got nuthin' yet					 */
	xprintf( 1, "compiling starters" );
	starters = pool_new( sizeof(trigger_t), NULL, NULL );
	for( s = start_strings; *s; ++s )	{
		trigger_t * const	t = pool_alloc( starters );

		xprintf( 2, "compiling starter '%s'", *s );
		t->s = *s;
		if( regcomp(
			&(t->re),
			t->s,
			(REG_EXTENDED|REG_ICASE)
		) )	{
			perror( "out of starter memory" );
			abort();
		}
	}
	xprintf( 1, "compiling enders" );
	enders = pool_new( sizeof(trigger_t), NULL, NULL );
	for( s = end_strings; *s; ++s )	{
		trigger_t * const	t = pool_alloc( enders );

		xprintf( 2, "compiling ender '%s'", *s );
		t->s = *s;
		if( regcomp(
			&(t->re),
			t->s,
			(REG_EXTENDED|REG_ICASE)
		) )	{
			perror( "out of ender memory" );
			abort();
		}
	}
	/* Host states: 0=looking, 1=ending				 */
	host_states = xmalloc( hosts_qty );
	memset( host_states, 0, hosts_qty );
	/* Iterate over all the entries, looking for a starter		 */
	xprintf( 1, "find starters" );
	iter = pool_iter_new( entries );
	for(
		e = pool_iter_next( iter );
		e;
		e = pool_iter_next( iter )
	)	{
		pool_iter_t *		subiter;
		trigger_t *		t;

		subiter = pool_iter_new(
			host_states[e->host_id] ? enders : starters
		);
		for(
			t = pool_iter_next( subiter );
			t;
			t = pool_iter_next( subiter )
		)	{
			regmatch_t	matches[10];

			if( !regexec(
				&t->re,
				e->resid,
				DIM(matches),
				matches,
				0
			) )	{
				/* Found a match			 */
				e->trigger = t;
				host_states[e->host_id] = 1;
				break;
			}
		}
		if( (e->trigger == NULL) && (subiter->pool == enders) ) {
			host_states[e->host_id] = 0;
		}
		pool_iter_free( &subiter );
	}
}
