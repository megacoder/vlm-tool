#ifndef	_STANZAS_H
#define	_STANZAS_H
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

#include <gcc-compat.h>

#include <vlm-tool.h>

typedef	struct	stanza_s	{
	trigger_t *	starters;	/* Rules to begin a region	 */
	trigger_t *	items;		/* Body rules for the region	 */
	uint16_t	flags;		/* Flags			 */
#define	STANZA_STOP	(1U<<0)		/* Rule match ends stanza	 */
	uint16_t	budget;		/* Region ends after line qty	 */
} stanza_t;

extern	stanza_t *	stanzas[];

#endif	/* _STANZAS_H */
