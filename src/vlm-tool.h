#ifndef	_VLM_TOOL_H
#define	_VLM_TOOL_H
#define	_GNU_SOURCE

typedef	struct	trigger_s	{
	char const *	s;		/* Original string		 */
	regex_t		re;		/* Compiled regular expression	 */
} trigger_t;

typedef	struct	entry_s	{
	trigger_t *	trigger;	/* We matched this trigger	 */
	char *		resid;		/* Everything after host name	 */
	size_t		host_id;	/* Index into hostname table	 */
	time_t		timestamp;	/* Date for entry		 */
} entry_t;

extern	unsigned	hosts_qty;
extern	pool_t *	entries;

#endif	/* _VLM_TOOL_H */
