#include <unistd.h>
#include <stdio.h>
#include <regex.h>
#include <getopt.h>
#include <string.h>
#include <malloc.h>

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
	/* Get out of Dodge						 */
	return( nonfatal ? 1 : 0 );
}
