#include <sys/types.h>
#include <unistd.h>
#include <regex.h>
#include <stdint.h>

#include <gcc-compat.h>
#include <vlm-tool.h>

#include <stanzas.h>

static	trigger_t	cut_starters[] =	{
	{ "---[ cut.*]---" },
	{ NULL }			/* Must be last			 */
};

static	trigger_t	cut_items[] =	{
	{ "---[.*]---" },
	{ NULL }			/* Must be last			 */
};

static	stanza_t	cut_stanza =	{
	cut_starters,			/* No starters yet		 */
	cut_items,			/* No items yet			 */
	STANZA_STOP,			/* No flags			 */
	0				/* No budget			 */
};

stanza_t *	stanzas[] = {
	&cut_stanza,
	NULL				/* Must be last			 */
};
