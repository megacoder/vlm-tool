#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <getopt.h>
#include <string.h>
#include <malloc.h>
#include <regex.h>

#include <builtins.h>

#define	TRIGGER_INCR	(256)		/* Grow table by this many	 */

static	char const *	me = "vlm-tool";
static	unsigned	nonfatal;
static	char const *	thumb = "+";
static	unsigned	list_triggers;
static	unsigned	mark_entries;
static	unsigned	load_builtin_rules;
static	char const *	ofile;
static	unsigned	show_rule;
static	unsigned	show_stats;
static	unsigned	colorize;
static	regex_t*	triggers;
static	size_t		triggerQty;
static	size_t		triggerPos;

int
main(
	int		argc,
	char *		argv[]
)
{
	int		c;

	/* Compute our process name					 */
	me = strrchr( argv[0], '/' ) + 1;
	if( me == (char *) 1 )	{
		me = argv[0];
	}
	/* Process command line						 */
	while( (c = getopt( argc, argv, "a:b:clmno:rst:" )) != EOF )	{
		switch( c )	{
		default:
			fprintf(
				stderr,
				"%s: not implemented yet [-%c]\n",
				me,
				c
			);
			++nonfatal;
			break;
		case '?':
			fprintf(
				stderr,
				"%s: unknown switch [-%c]\n",
				me,
				c
			);
			++nonfatal;
			break;
		case 'c':
			colorize = 1;
			break;
		case 'l':
			list_triggers = 1;
			break;
		case 'm':
			mark_entries = 1;
			break;
		case 'n':
			load_builtin_rules = 0;
			break;
		case 'o':
			ofile = optarg;
			break;
		case 'r':
			show_rule = 1;
			break;
		case 's':
			show_stats = 1;
			break;
		case 't':
			thumb = optarg;
			break;
		}
	}
	/* Compile internal triggers unless we are forbidden		 */
	if( load_builtin_rules )	{
		char const * *	builtin;

		for( builtin = builtin_triggers; *builtin; ++builtin )	{
			char		buffer[ BUFSIZ ];

			/* Skip deleted builtin rules			 */
			if( *builtin == (char *) -1 )	{
				continue;
			}
			/* Wrap trigger for pre- and post-regions	 */
			if( snprintf(
				buffer,
				sizeof(buffer),
				"^(.*)(%s)(.*)$",
				*builtin
			) > sizeof(buffer) )	{
				fprintf(
					stderr,
					"%s: rule too long [%s]\n",
					me,
					*builtin
				);
				exit( 1 );
				/*NOTREACHED*/
			}
			/* Compile the resulting trigger		*/
			if( triggerPos >= triggerQty )	{
				triggerQty += TRIGGER_INCR;
				triggers = realloc(
					triggers,
					triggerQty * sizeof( triggers[0] )
				);
			}
			if( regcomp(
				triggers + triggerPos,
				buffer,
				(REG_EXTENDED|REG_ICASE)
			) == -1 )	{
				fprintf(
					stderr,
					"%s: trigger [%s] failed to compile.\n",
					me,
					buffer
				);
				exit( 1 );
			}
			++triggerPos;
		}
	}
	/* Get out of Dodge						 */
	return( nonfatal ? 1 : 0 );
}
