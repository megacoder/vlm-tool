#include <vlm-tool.h>

#include <stanzas.h>

/*
 *------------------------------------------------------------------------
 * Oops message highlighting
 *------------------------------------------------------------------------
 */

static	trigger_t	oops_starters[] =	{
	{ "---[ cut here ]---" },
	{ NULL }			/* Must be last			 */
};

static	trigger_t	oops_items[] =	{
	{ "---[ end trace .* ]---" },
	{ NULL }			/* Must be last			 */
};

static	stanza_t	oops_stanza =	{
	oops_starters,
	oops_items,
	STANZA_STOP,
	50
};

/*
 *------------------------------------------------------------------------
 * Kernel errors which show a stacktrace
 *------------------------------------------------------------------------
 */

static	trigger_t	trace_starters[] =	{
	{ "unable to handle" },
	{ "call trace:" },
	{ "kobject_add failed for.*EXIST" },
	{ NULL }			/* Must be last			 */
};

static	trigger_t	trace_items[] =	{
	{ "[[]<[0-9a-fA-F]*>[]](.*)?" },
	{ NULL }			/* Must be last			 */
};

static	stanza_t	trace_stanza =	{
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
