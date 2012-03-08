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
#define	HOSTS_INCR	(4096/sizeof(char *))	/* Exactly one VM page	 */
#define	ENTRIES_INCR	(102400)	/* Entries table increment	 */

typedef	struct	trigger_s	{
	char const *	s;		/* Original rule string		 */
	regex_t		re;		/* Compiled regex		 */
} trigger_t;

typedef	struct	entry_s	{
	time_t		timestamp;
	unsigned	host_id;
	int		marked;
	char *		resid;
	trigger_t *	trigger;
} entry_t;

static	char const *	me = "vlm_tool";
static	unsigned	nonfatal;
static	char const *	thumb = "-";
static	unsigned	list_triggers;
static	unsigned	mark_entries;
static	unsigned	load_builtin_rules = 1;
static	char const *	ofile;
static	unsigned	show_rules;
static	unsigned	show_stats;
static	unsigned	colorize;
static	trigger_t *	triggers;
static	size_t		triggerQty;
static	size_t		triggerPos;
static	int		year;
static	unsigned	hostsQty;
static	unsigned	hostsPos;
static	char * *	hosts;
static	unsigned	entriesQty;
static	unsigned	entriesPos;
static	entry_t *	entries;
static	unsigned	debug;

static char const	sgr_red[] =	{
	"\033[1;31m"
};
static char const	sgr_reset[] =	{
	"\033[0m"
};

static	void *		_inline
xstrdup(
	char const * const	s
)
{
	void * const	retval = strdup( s );

	if( !retval )	{
		fprintf( stderr, "%s: out of memory.\n", me );
		exit(1);
	}
	return( retval );
}

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

static	unsigned
add_host(
	char const * const	name
)
{
	unsigned	retval;

	do	{
		/* See if we know this one already			 */
		for( retval = 0; retval < hostsPos; ++retval )	{
			if( !strcmp( name, hosts[retval] ) )	{
				return( retval );
			}
		}
		/* Add new host to table				 */
		if( hostsPos >= hostsQty )	{
			hostsQty += HOSTS_INCR;
			hosts = realloc(
				hosts,
				sizeof(hosts[0]) * hostsQty
			);
		}
		retval = hostsPos++;
		hosts[ retval ] = xstrdup( name );
	} while( 0 );
	return( retval );
}

static	void
add_trigger(
	char const * const	rule
)
{
	/* Grow table if needed						 */
	if( triggerPos >= triggerQty )	{
		triggerQty += TRIGGER_INCR;
		triggers = realloc(
			triggers,
			triggerQty * sizeof( triggers[0] )
		);
	}
	/* Compile the resulting trigger				 */
	if( debug > 0 )	{
		printf( "Adding rule '%s'.\n", rule );
	}
	triggers[triggerPos].s = xstrdup( rule );
	if( regcomp(
		&triggers[triggerPos].re,
		rule,
		(REG_EXTENDED|REG_ICASE)
	) == -1 )	{
		fprintf(
			stderr,
			"%s: trigger [%s] failed to compile.\n",
			me,
			rule
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
add_entry(
	entry_t *	e
)
{
	/* Grow table if needed						 */
	if( entriesPos >= entriesQty )	{
		/* Realloc can get expensive, so grab much gusto	 */
		entriesQty += ENTRIES_INCR;
		entries = realloc(
			entries,
			entriesQty * sizeof(entries[0])
		);
		if( !entries )	{
			fprintf(
				stderr,
				"%s: out of entry memory.\n",
				me
			);
			exit(1);
			/*NOTREACHED*/
		}
	}
	*(entries + entriesPos++) = *e;
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
		entry_t		e;
		size_t		trigger;
		char		color_buffer[ BUFSIZ * 3 ];

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
		e.marked = -1;
		if( calc_timestamp( ts, &e.timestamp ) )	{
			/* Badly-formatted timestamp, ignore line	 */
			continue;
		}
		e.trigger = NULL;
		e.host_id = add_host( host );/* Remember host		 */
		/* Pick a matching trigger				 */
		for( trigger = 0; trigger < triggerPos; ++trigger )	{
			static	size_t	rule_no;
			regmatch_t	matches[ 10 ];

			if( !regexec(
				&triggers[rule_no].re,
				resid,
				DIM( matches ),
				matches,
				0
			) )	{
				regmatch_t * const	mid = matches+0;

				e.trigger = triggers+rule_no;
				e.marked = rule_no;
				if( (mid->rm_so != -1) && colorize )	{
					char *	bp;

					bp = memcpy(
						color_buffer,
						resid,
						mid->rm_so
					);
					bp += mid->rm_so;
					strcpy( bp, sgr_red );
					bp += strlen( sgr_red );
					memcpy( bp, resid+mid->rm_so, mid->rm_eo - mid->rm_so );
					bp += (mid->rm_eo - mid->rm_so);
					strcpy( bp, sgr_reset );
					bp += strlen( sgr_reset );
					strcpy( bp, resid + mid->rm_eo );
					resid = color_buffer;
				}
				break;
			}
			rule_no = (rule_no + 1) % triggerPos;
		}
		if( (e.marked > -1) | mark_entries )	{
			e.resid = xstrdup( resid );
			add_entry( &e );
		}
	}
}

static	int
compar(
	const void *	l,
	const void *	r
)
{
	const entry_t *	le = l;
	const entry_t *	re = r;
	int		retval;

	/* Order by time then by host					 */
	if( le->timestamp < re->timestamp )	{
		retval = -1;
	} else if( le->timestamp == re->timestamp )	{
		retval = strcmp( hosts[le->host_id], hosts[re->host_id] );
	} else	{
		retval = 1;
	}
	return( retval );
}

static	void
sgr_host(
	unsigned	host_id
)
{
	printf( "\033[1;%dm", (31 + (host_id % 6)) );
}

static	void
print_entries(
	void
)
{
	entry_t * const	Lentry = entries + entriesPos;
	entry_t *	e;
	size_t		hlen;
	unsigned	host_id;
	char *		no_thumb;

	/* Calculate a blank thumb for un-marked entries		 */
	no_thumb = xstrdup( thumb );
	memset( no_thumb, ' ', strlen(no_thumb) );
	/* Find longest host name we've kept				 */
	hlen = 0;
	for( host_id = 0; host_id < hostsPos; ++host_id )	{
		hlen = max( hlen, strlen( hosts[host_id] ) );
	}
	/* Output the retained lines					 */
	for( e = entries; e < Lentry; ++e )	{
		static const char	fmt[] = "%-*s ";
		struct tm *	tm;

		/* First, the thumb (if marked)				 */
		if( mark_entries )	{
			printf( "%s ", e->marked == -1 ? no_thumb : thumb );
		}
		/* Special case: show rule if asked			 */
		if( show_rules )	{
			printf(
				"%-15.15s ",
				e->trigger ? e->trigger->s : ""
			);
		}
		/* Second, the date					 */
		tm = gmtime( &e->timestamp );
		printf( "%.15s ", asctime(tm)+4 );
		/* Third, the host name					 */
		if( colorize )	{
			sgr_host( e->host_id );
			printf( fmt, (int) hlen, hosts[e->host_id] );
			printf( "%s", sgr_reset );
		} else	{
			printf( fmt, (int) hlen, hosts[e->host_id] );
		}
		/* Now, the remainder of the text			 */
		printf( "%s\n", e->resid );
	}
}

static	void
do_file(
	char const * const	fn
)
{
	FILE *		fyle;
	int		(*closer)( FILE * );
	char *		bp;

	bp = strrchr( fn, '.' );
	if( bp && !strcmp( bp, ".gz" ) )	{
		char	cmd[ BUFSIZ ];
		int	n;

		n = snprintf( cmd, sizeof(cmd), "/bin/zcat -- %s", fn );
		if( n > sizeof(cmd) )	{
			perror( fn );
			exit( 1 );
		}
		fyle = popen( cmd, "r" );
		closer = pclose;
	} else if( bp && !strcmp( bp, ".bz2" ) )	{
		char	cmd[ BUFSIZ ];
		int	n;

		n = snprintf( cmd, sizeof(cmd), "/usr/bin/bzcat -- %s", fn );
		if( n > sizeof(cmd) )	{
			perror( fn );
			exit( 1 );
		}
		fyle = popen( cmd, "r" );
		closer = pclose;
	} else if( bp && !strcmp( bp, ".xz" ) )	{
		char	cmd[ BUFSIZ ];
		int	n;

		n = snprintf( cmd, sizeof(cmd), "/usr/bin/xzcat -- %s", fn );
		if( n > sizeof(cmd) )	{
			perror( fn );
			exit( 1 );
		}
		fyle = popen( cmd, "r" );
		closer = pclose;
	} else	{
		fyle = fopen( fn, "rt" );
		closer = fclose;
	}
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
		if( (*closer)( fyle ) )	{
			perror( optarg );
			++nonfatal;
		}
	}
}

static	void
dump_rules(
	void
)
{
	trigger_t * const	last = triggers + triggerPos;
	trigger_t *		trigger;

	for( trigger = triggers; trigger < last; ++trigger )	{
		printf( "%s\n", trigger->s );
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
	while( (c = getopt( argc, argv, "Xa:b:clmno:rt:vy:" )) != EOF )	{
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
		case 'X':
			++debug;
			break;
		case 'a':
			add_trigger( optarg );
			break;
		case 'b':
			bulk_load( optarg );
			break;
		case 'c':
			colorize = 1;
			mark_entries = 1;
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
			mark_entries = 1;
			break;
		case 's':
			show_stats = 1;
			break;
		case 't':
			thumb = optarg;
			break;
		case 'v':
			printf( "%s %s\n", me, VERSION );
			exit(0);
			/*NOTREACHED*/
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
	/* List our rules						 */
	if( debug > 0 )	{
		trigger_t * const	last = triggers+triggerPos;
		trigger_t *		trigger;

		printf( "%u Triggers:\n", triggerPos );
		for( trigger = triggers; trigger < last; ++trigger )	{
			puts( trigger->s );
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
	/* Here we go							 */
	if( optind < argc )	{
		/* Named files						 */
		while( optind < argc )	{
			optarg = argv[ optind++ ];
			if( !strcmp( optarg, "-" ) )	{
				process( stdin );
			} else	{
				do_file( optarg );
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
					do_file( path );
				}
			}
			closedir( dir );
		}
	}
	/* Sort retained entries					 */
	qsort(
		entries,
		entriesPos,
		sizeof(entries[0]),
		compar
	);
	/* Print results						 */
	print_entries();
	/* Get out of Dodge						 */
	return( nonfatal ? 1 : 0 );
}
