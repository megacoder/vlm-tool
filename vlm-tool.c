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
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <errno.h>

#include <gcc-compat.h>
#include <builtins.h>

#define	TRIGGER_INCR	(256)		/* Grow table by this many	 */

typedef	struct	entry_s	{
	time_t		timestamp;
	char *		line;
	unsigned	matched;
} entry_t;

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
static	int		year;

static	int
calc_timestamp(
	char *		timestamp,
	time_t *	t
)
{
	static char const months[12][4] =	{
		{ "Jan\0" },
		{ "Feb\0" },
		{ "Mar\0" },
		{ "Apr\0" },
		{ "Jun\0" },
		{ "Jul\0" },
		{ "Aug\0" },
		{ "Sep\0" },
		{ "Oct\0" },
		{ "Nov\0" },
		{ "Dec\0" },
	};
	int		retval;
	char		mmddhhmmss[16];
	struct tm	tm;

	retval = -1;
	do	{
		/* 11111						 */
		/* 012345678901234					 */
		/* MMM DD HH:MM:SS					 */
		memcpy( mmddhhmmss, timestamp, 15 );
		mmddhhmmss[ 3] = '\0';
		mmddhhmmss[ 6] = '\0';
		mmddhhmmss[ 9] = '\0';
		mmddhhmmss[12] = '\0';
		tm.tm_sec      = strtoul( mmddhhmmss+13, NULL, 10 );
		tm.tm_min      = strtoul( mmddhhmmss+10, NULL, 10 );
		tm.tm_hour     = strtoul( mmddhhmmss+7, NULL, 10 );
		tm.tm_mday     = strtoul( mmddhhmmss+4, NULL, 10 );
		/* Pick the month out of the line-up			 */
		for( tm.tm_mon = 0; tm.tm_mon < 12; tm.tm_mon += 1 )	{
			if(!strcmp( mmddhhmmss, months[tm.tm_mon] ) )	{
				break;
			}
		}
		/* Fill in intuited year				 */
		tm.tm_year = year - 1900;
		tm.tm_isdst = -1;
		/* Convert to time_t					 */
		*t = mktime( &tm );
		/* Tell if we've screwed up the date			 */
		if( *t != (time_t) -1 )	{
			retval = 0;
		}
	} while( 0 );
	return( retval );
}

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

static	void
process(
	FILE * const	fyle		/* Syslogd output to scan	 */
)
{
	char		buf[ BUFSIZ ];	/* Incoming line goes here	 */

	while( fgets( buf, sizeof(buf), fyle ) )	{
		int const	l = strlen( buf );
		char *		bp;
		char *		ts;
		char *		host;
		char *		resid;
		time_t		t;

		/* Drop trailing whitespace				 */
		for( bp = buf + l; (bp > buf) && isspace(bp[-1]); --bp ) {
			bp[-1] = '\0';
		}
		/* Isolate timestamp and hostname			 */
		ts = buf;
		buf[15] = '\0';
		host = buf + 16;
		for( bp = host+1; *bp && !isspace( *bp ); ++bp );
		*bp = '\0';
		resid = bp + 1;
		/* Convert info to timestamp				 */
		if( calc_timestamp( ts, &t ) )	{
			/* Badly-formatted timestamp, ignore line	 */
			continue;
		}
		printf( "[%.24s] %s %-15s %s\n", ctime(&t), ts, host, resid );
	}
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
	while( (c = getopt( argc, argv, "a:b:clmno:rst:y:" )) != EOF )	{
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
		case 'y':
			year = strtol( optarg, NULL, 10 );
			break;
		}
	}
	/* Choose current year unless we've be told otherwise		 */
	if( !year )	{
		time_t const	now = time( NULL );
		struct tm const *	tm = localtime(&now);
		year = tm->tm_year + 1900;
	}
	printf( "The year is %d.\n", year );
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
	/* Here we go							 */
	if( optind < argc )	{
		/* Named files						 */
		while( optind < argc )	{
			optarg = argv[ optind++ ];
			if( !strcmp( optarg, "-" ) )	{
				process( stdin );
			} else	{
				FILE * const	fyle = fopen( optarg, "rt" );
				if( !fyle )	{
					fprintf(
						stderr,
						"%s: cannot read [%s].\n",
						me,
						optarg
					);
					++nonfatal;
				} else	{
					process( fyle );
					if( fclose( fyle ) )	{
						perror( optarg );
						++nonfatal;
					}
				}
			}
		}
	} else	{
		/* Nothing specified, read "/var/log/messages*" 	 */
		static char const	vdir[] = "/var/log";
		DIR *		dir;

		dir = opendir( vdir );
		if( dir )	{
			struct dirent *	de;

			while( (de = readdir( dir )) != NULL )	{
				if( !strncmp( de->d_name, "messages", 8 ) ) {
					char	path[ PATH_MAX ];
					snprintf(
						path,
						sizeof( path ),
						"%s/%s",
						vdir,
						de->d_name
					);
					printf( "pretend [%s]\n", path );
				}
			}
			closedir( dir );
		}
	}
	/* Get out of Dodge						 */
	return( nonfatal ? 1 : 0 );
}
