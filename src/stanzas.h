#ifndef	_STANZAS_H
#define	_STANZAS_H
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

#include <vlm-tool.h>

typedef	struct	stanza_s	{
	char const *	name;		/* Name of the stanza		 */
	char * *	starters;	/* Rules to begin a region	 */
	char * *	items;		/* Body rules for the region	 */
	uint16_t	flags;		/* Flags			 */
#define	STANZA_CONT	(0U<<0)		/* Rule match extends stanza	 */
#define	STANZA_STOP	(1U<<0)		/* Rule match ends stanza	 */
	uint16_t	budget;		/* Region ends after line qty	 */
	/* Keep overhead stuff below, so we don't bork the inits later	 */
	pool_t *	starter_pool;	/* Pool of stanza start rules	 */
	pool_t *	item_pool;	/* Pool of stanza body rules	 */
} stanza_t;

void		stanza_setup( void );	/* Must be first call		 */
stanza_t *	stanza_find( entry_t *, int ); /* Entry match any rule?	 */

#endif	/* _STANZAS_H */
