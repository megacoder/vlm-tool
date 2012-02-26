#include <sys/types.h>
#include <ctype.h>
#include <getopt.h>
#include <malloc.h>
#include <regex.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <builtins.h>

#define	TRIGGER_INCR	(256)		/* Grow table by this many	 */

static	char const *	me = "vlm-tool";
static	unsigned	nonfatal;
static	char const *	thumb = "+";
static	unsigned	list_triggers;
static	unsigned	mark_entries;
static	unsigned	load_builtin_rules;
static	char const *	ofile;
static	unsigned	show_rules;
static	unsigned	show_stats;
static	unsigned	colorize;
static	regex_t*	triggers;
static	size_t		triggerQty;
static	size_t		triggerPos;

static	void
add_trigger(
	char const * const	rule
)
{
	char	buffer[ BUFSIZ ];
	/* Wrap trigger for pre- and post-regions			 */
	if( snprintf(
		buffer,
		sizeof( buffer ),
		"^(.*)(%s)(.*)$",
		rule
	) > sizeof(buffer) )	{
		fprintf(
			stderr,
			"%s: rule too long [%s]\n",
			me,
			rule
		);
		exit( 1 );
		/* NOTREACHED						 */
	}
	/* Grow table if needed						 */
	if( triggerPos >= triggerQty )	{
		triggerQty += TRIGGER_INCR;
		triggers = realloc(
			triggers,
			triggerQty * sizeof( triggers[0] )
		);
	}
	/* Compile the resulting trigger				 */
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

static	void
bulk_load(
	char const * const	fn
)
{
	FILE *		fyle;

	fyle = fopen( fn, "rt" );
	if( !fyle )	{
		fprintf(
			stderr,
			"%s: cannot read bulk file [%s].\n",
			me,
			fn
		);
		exit(1);
		/*NOTREACHED*/
	}
	for( ; ; )	{
		char	buffer[ 256 ];
		char *	bp;

		/* Check for EOF					 */
		if( !fgets( buffer, sizeof(buffer), fyle ) )	{
			break;
		}
		/* Drop line terminator and trailing whitespace		 */
		for(
			bp = buffer + strlen(buffer);
			(bp > buffer) && isspace(bp[-1]);
			--bp
		)	{
			bp[-1] = '\0';
		}
		/* Step over leading whitespace				 */
		for( bp = buffer; *bp && isspace( *bp ); ++bp );
		/* What's left is a trigger				 */
		add_trigger( bp );
	}
	fclose( fyle );
}

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
		case 'a':
			add_trigger( optarg );
			break;
		case 'b':
			bulk_load( optarg );
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
			show_rules = 1;
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
			/* Skip deleted builtin rules			 */
			if( *builtin != (char *) -1 )	{
				add_trigger( *builtin );
			}
		}
	}
	/* Redirect stdout if asked					 */
	if( ofile )	{
		if( freopen( ofile, "wt", stdout ) != stdout )	{
			fprintf(
				stderr,
				"%s: cannot redirect output [%s].\n",
				me,
				ofile
			);
			exit(1);
		}
	}
	/* Show rules that we have					 */
	if( show_rules )	{
		puts( "Dunno how yet." );
	}
	/* Get out of Dodge						 */
	return( nonfatal ? 1 : 0 );
}
