#define	_XOPEN_SOURCE
#define	_GNU_SOURCE

#include <config.h>

#include <sys/types.h>
#include <ctype.h>
#include <getopt.h>
#include <malloc.h>
#if	HAVE_LIBPCRE
#  include <pcreposix.h>
#else /* !NOPE */
#  include <regex.h>
#endif /* !NOPE */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <math.h>

#include <gcc-compat.h>
#include <builtins.h>
#include <pool.h>
#include <xprintf.h>
#include <x-funcs.h>
#include <post-process.h>
#include <utils.h>

#include <vlm-tool.h>

#define	SEC_PER_MINUTE	(60UL)
#define	SEC_PER_HOUR	(SEC_PER_MINUTE * 60UL)
#define	SEC_PER_DAY	(SEC_PER_HOUR * 24UL)

typedef	struct	log_stats_s	{
	unsigned long	read;
	unsigned long	dropped;
	unsigned long	flattened;
	unsigned long	output;
	unsigned long	poorly_formed;
} log_stats_t;

typedef	struct	timekind_s	{
	char const *	name;
	size_t const	len;
	size_t const	extra;
	char const *	in;
	char const *	out;
} timekind_t;

typedef	enum	timekind_e	{
	TIMEKIND_USA	= 0,
	TIMEKIND_ISO
} timekind;

static	timekind_t	timekinds[] =	{
	[TIMEKIND_USA] =	{
		"USA",	16,	0,
		"%B%n%d%n%T%n",
		"%b %d %T"
	},
	[TIMEKIND_ISO] =	{
		"ISO",	33,	13,
		"%Y-%m-%dT%H:%M:%S",
		"%Y-%m-%dT%H:%M:%S%z"
	},
};
static	size_t		Ntimekinds = sizeof( timekinds ) / sizeof( timekinds[0] );

static	char const *	me = "vlm_tool";
static	unsigned	nonfatal;
static	char const *	thumb = "- ";
static	char *		no_thumb;
static	unsigned	list_triggers;
static	unsigned	mark_entries;
static	unsigned	load_builtin_rules = 1;
static	char const *	ofile;
static	unsigned	show_rules;
static	unsigned	colorize;
static	pool_t *	triggers;
static	int		year;
unsigned		hosts_qty;
static	unsigned	hostsPos;
static	char * *	hosts;
pool_t *		entries;
static	size_t		entries_qty;
static	entry_t * *	flat_entries;
static	unsigned	debug;
static	size_t		hlen;
static	pool_t *	ignores;
static	int		want_lineno;
static	log_stats_t	log_stats;
static	unsigned	do_stats;
static	time_t		gap_threshold = 10 * 60;
static	unsigned	do_gap;
static	timekind	date_kind = TIMEKIND_USA;
static	char const *	about;
static	time_t		incident;
static	time_t		window;
static	time_t		delta_hours;

static char const	sgr_red[] =	{
	"\033[1;31;22;47m"		/* Bright red text, dirty white bg */
};
static char const	sgr_reset[] =	{
	"\033[0m"
};

static	char	*
calc_timestamp(
	char const *	timestamp,
	time_t *	t
)
{
	char *		retval;

	retval = NULL;
	do	{
		timekind_t *	tk = timekinds + date_kind;
		struct tm	tm;

		memset( &tm, 0, sizeof( tm ) );
		tm.tm_year = year;
		retval = strptime( timestamp, tk->in, &tm );
		if( ! retval )	{
			fprintf(
				stderr,
				"%s: unknown date format.\n",
				me
			);
			fprintf(
				stderr,
				"%s: fmt '%s'\n",
				me,
				tk->in
			);
			fprintf(
				stderr,
				"%s: dat '%*s'\n",
				me,
				(int) tk->len,
				timestamp
			);
			break;
		}
		/* Convert to time_t					 */
#if	0
		if( tm.tm_year > 1900 )	{
			tm.tm_year -= 1900;
		}
#endif	/* NOPE */
		*t = mktime( &tm );
		/* Tell if we've screwed up the date			 */
		if( *t == (time_t) -1 )	{
			retval = NULL;
		}
		retval += tk->extra;
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
		if( hostsPos >= hosts_qty )	{
			static	size_t const	incr = 32;

			hosts_qty += incr;
			hosts = realloc(
				hosts,
				sizeof(hosts[0]) * hosts_qty
			);
		}
		retval = hostsPos++;
		hosts[ retval ] = xstrdup( name );
		hlen = max(hlen, strlen( name ));
		xprintf( 2, "added host #%u '%s'", retval, hosts[retval] );
	} while( 0 );
	return( retval );
}

static	void
add_pattern(
	pool_t * const		pool,
	char const * const	rule
)
{
	trigger_t * const	t = pool_alloc( pool );

	trigger_compile( rule, t );
}

static	void
bulk_load(
	pool_t * const		pool,
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
		add_pattern( pool, xstrdup( bp ) );
	}
	fclose( fyle );
}

static	void
announce_ignored(
	void
)
{
	trigger_t *	trigger;
	pool_iter_t *	iter;

	iter = pool_iter_new( ignores );
	do	{
		int		others;

		for(
			others = 0, trigger = pool_iter_next( iter );
			trigger;
			trigger = pool_iter_next( iter )
		)	{
			if( !others )	{
				printf(
					"# Lines matching these patterns "
					"have been ignored:\n"
				);
				++others;
			}
			printf(
				"# %s\n",
				trigger->s
			);
		}
	} while( 0 );
	pool_iter_free( &iter );
}

static	void
process(
	FILE * const	fyle		/* Syslogd output to scan	 */
)
{
	char		buf[ BUFSIZ ];	/* Incoming line goes here	 */
	time_t const	future = time( NULL ) + (delta_hours * 60 * 60);

	while( fgets( buf, sizeof(buf), fyle ) )	{
		int const	l = strlen( buf );
		char *		bp;
		char *		host;
		char *		resid;
		trigger_t *	fired;
		int		keep;
		pool_iter_t *	iter;
		time_t		when;

		/* Count this line					 */
		log_stats.read += 1;
		/* Drop trailing whitespace				 */
		for( bp = buf + l; (bp > buf) && isspace(bp[-1]); --bp ) {
			bp[-1] = '\0';
		}
		/* Convert info to timestamp				 */
		host = calc_timestamp( buf, &when );
		/* Bad date, show it first in list			 */
		if( !host )	{
			log_stats.poorly_formed += 1;
			when = (time_t) 0;
			host = buf + 16;
		}
		xprintf(
			3,
			"when=%lu, incident=%lu, window=%lu",
			when,
			incident,
			window
		);
		if( when > future )	{
			/*
			 * Looks as if /var/log/messages has wrapped and
			 * what we think is a current line is actually
			 * from a year ago.  Message entries do not include
			 * the year, so this entry is from another year.
			 */
			xprintf(
				3,
				"when=%lu, future=%lu",
				when,
				future
			);
			continue;
		}
		if( window && (abs(when - incident) > window) )	{
			continue;
		}
		for( resid = host; *resid && !isspace( *resid ); ++resid );
		if( *resid )	{
			(*resid++) = '\0';
		}
		/* Qualify the line					 */
		iter = pool_iter_new( ignores );
		fired = NULL;
		do	{
			for(
				fired = pool_iter_next( iter );
				fired;
				fired = pool_iter_next( iter )
			)	{
				if( trigger_match( resid, fired, NULL ) ) {
					/* Leave loop early		 */
					break;
				}
			}
		} while( 0 );
		pool_iter_free( &iter );
		if( fired )	{
			/* Do not want this line at all			 */
			log_stats.dropped += 1;
			continue;
		}
		/* Pick a matching trigger				 */
		keep = mark_entries;
		iter = pool_iter_new( triggers );
		do	{
			/* Find a trigger that fits			 */
			for(
				fired = pool_iter_next( iter );
				fired;
				fired = pool_iter_next( iter )
			)	{
				if( trigger_match( resid, fired, NULL ) ) {
					keep  = 1;
					break;
				}
			}
		} while( 0 );
		pool_iter_free( &iter );
		/* Remember this entry if we're keeping them		 */
		if( keep )	{
			entry_t * const	e = pool_alloc( entries );

			e->host_id    = add_host( host );
			e->timestamp  = when;
			e->trigger    = fired;
			e->resid      = xstrdup( resid );
			entries_qty  += 1;
		}
	}
}

static	int
compar(
	const void *	l,
	const void *	r
)
{
	entry_t * const *	le = l;
	entry_t * const *	re = r;
	int		retval;

	/* Order by time then by host					 */
	if( le[0]->timestamp < re[0]->timestamp )	{
		retval = -1;
	} else if( le[0]->timestamp == re[0]->timestamp )	{
		retval = strcmp( hosts[le[0]->host_id], hosts[re[0]->host_id] );
	} else	{
		retval = 1;
	}
	return( retval );
}

static	void
flatten_and_sort_entries(
	void
)
{
	pool_iter_t *	iter;

	flat_entries = xmalloc( entries_qty * sizeof( flat_entries[0] ) );
	iter	     = pool_iter_new( entries );
	do	{
		entry_t * *	etp;
		entry_t *	e;

		/* Build index of entry addresses			 */
		for(
			etp = flat_entries,
			e = pool_iter_next( iter );
			e;
			e = pool_iter_next( iter ),
			++etp
		)	{
			*etp = e;
		}
		/* Order table of entry addresses			 */
		xprintf( 1, "Sorting %u entries", (unsigned) entries_qty );
		qsort(
			flat_entries,
			entries_qty,
			sizeof(flat_entries[0]),
			compar
		);
		log_stats.flattened = entries_qty;
	} while( 0 );
	pool_iter_free( &iter );
}

static	void
sgr_host(
	unsigned	host_id
)
{
	/* Color 30 is black, up to color 37 which is white		 */
	printf( "\033[1;%dm", (31 + ((host_id+1) % 6)) );
}

static	void
print_one_entry(
	entry_t * const		e
)
{
	static const char	fmt[] = "%-*s ";
	timekind_t const * const tk = timekinds + date_kind;
	struct tm *		tm;
	char			when[ 80 ];

	/* Count it							 */
	log_stats.output += 1;
	/* Mind the gap							 */
	if( do_gap )	{
		static	time_t	last;

		if( last )	{
			time_t		delta;

			delta = e->timestamp - last;
			if( delta >= gap_threshold )	{
				char	buffer[ BUFSIZ + 1 ];
				char *	bp;
				time_t	days;
				time_t	hours;
				time_t	minutes;
				time_t	seconds;

				seconds = delta;
				days = seconds / SEC_PER_DAY;
				seconds -= (days * SEC_PER_DAY);
				hours = seconds / SEC_PER_HOUR;
				seconds -= (hours * SEC_PER_HOUR);
				minutes = seconds / SEC_PER_MINUTE;
				seconds -= (minutes * SEC_PER_MINUTE);
				bp = buffer;
				if( days )	{
					bp += sprintf( bp, " %lu-day", (unsigned long) days );
				}
				if( hours )	{
					bp += sprintf( bp, " %lu-hour", (unsigned long) hours );
				}
				if( minutes )	{
					bp += sprintf( bp, " %lu-minute", (unsigned long) minutes );
				}
				if( seconds )	{
					bp += sprintf( bp, " %lu-second", (unsigned long) seconds );
				}
				if( bp == buffer )	{
					bp += sprintf( bp, " %lu-second", (unsigned long) delta );
				}
				printf( "\n%s***%s gap detected. ***\n\n",
					no_thumb,
					buffer
				);
			}
		}
		last = e->timestamp;
	}
	/* First, the thumb (if marked)					 */
	if( mark_entries )	{
		printf(
			"%s",
			e->trigger == NULL ? no_thumb : thumb
		);
	}
	/* Output a line number if we were asked nicely			 */
	if( want_lineno )	{
		static	unsigned long	lineno;
		static	unsigned int	width;

		if( !width )	{
			width = (unsigned int) log10( entries_qty );
		}
		printf( "%*lu ", width, ++lineno );
	}
	/* Special case: show rule if asked				 */
	if( show_rules )	{
		printf(
			"%-15.15s ",
			e->trigger ? e->trigger->s : ""
		);
	}
	/* Second, the date						 */
	tm = localtime( &e->timestamp );
	strftime(
		when,
		sizeof( when ) - 1,
		(char *) tk->out ? tk->out : tk->in,
		tm
	);
	printf(
		"%s ",
		when
	);
	/* Third, the host name					 */
	if( colorize )	{
		sgr_host( e->host_id );
		printf( fmt, (int) hlen, hosts[e->host_id] );
		printf( "%s", sgr_reset );
	} else	{
		printf( fmt, (int) hlen, hosts[e->host_id] );
	}
	/* Now, the remainder of the text			 */
	if( colorize && (e->trigger != NULL) )	{
		regmatch_t		matches[10];

		if( !regexec(
			&e->trigger->re,
			e->resid,
			DIM(matches),
			matches,
			0
		) )	{
			regmatch_t * const	rm = matches+0;
			char * const		resid = e->resid;

			if( asprintf(
				&e->resid,
				"%.*s" "%s%.*s%s" "%s",
				rm->rm_so,
				resid,
				sgr_red,
				rm->rm_eo - rm->rm_so,
				resid + rm->rm_so,
				sgr_reset,
				resid + rm->rm_eo
			) != -1 )	{
				free( resid );
			} else	{
				e->resid = resid;
			}
		}
	}
	printf( "%s\n", e->resid );
}

static	void
print_entries(
	void
)
{
	size_t			i;

	/* Calculate a blank thumb for un-marked entries		 */
	no_thumb = xstrdup( thumb );
	memset( no_thumb, ' ', strlen(no_thumb) );
	/* Iterate over the kept entries, printing all of them		 */
	xprintf( 1, "Listing %u entries", (unsigned) entries_qty );
	for( i = 0; i < entries_qty; ++i )	{
		entry_t * const		e = flat_entries[i];

		print_one_entry( e );
	}
}

static	int
has_ext(
	char const * const	fn,
	char const * const	ext
)
{
	int			retval;

	retval = 0;
	do	{
		char const *	bn;
		char const *	ep;

		bn = strrchr( fn, '/' );
		if( bn )	{
			++bn;
		} else	{
			bn = fn;
		}
		ep = strrchr( bn, '.' );
		if( ep && !strcmp( ep, ext ) )	{
			retval = 1;
		}
	} while( 0 );
	return( retval );
}

static	void
do_file(
	char const * const	fn
)
{
	FILE *		fyle;
	int		(*closer)( FILE * );

	xprintf( 1, "Processing file '%s'", fn );
	if( has_ext( fn, ".gz" ) )	{
		char	cmd[ BUFSIZ ];
		int	n;

		n = snprintf( cmd, sizeof(cmd), "/bin/zcat -- %s", fn );
		if( n > sizeof(cmd) )	{
			perror( fn );
			exit( 1 );
		}
		fyle = popen( cmd, "r" );
		closer = pclose;
	} else if( has_ext( fn, ".bz2" ) )	{
		char	cmd[ BUFSIZ ];
		int	n;

		n = snprintf( cmd, sizeof(cmd), "/usr/bin/bzcat -- %s", fn );
		if( n > sizeof(cmd) )	{
			perror( fn );
			exit( 1 );
		}
		fyle = popen( cmd, "r" );
		closer = pclose;
	} else if( has_ext( fn, ".xz" ) )	{
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
			fn
		);
		++nonfatal;
	} else	{
		if( setvbuf( fyle, NULL, _IOFBF, getpagesize() * 16 ) )	{
			fprintf(
				stderr,
				"%s: failed to set buffer size"
				"; continuing bravely onward.\n",
				me
			);
		}
		process( fyle );
		if( (*closer)( fyle ) )	{
			perror( optarg );
			++nonfatal;
		}
	}
}

#if	0
static	void
dump_rules(
	void
)
{
	pool_iter_t *		iter;

	iter = pool_iter_new( triggers );
	do	{
		trigger_t *	t;

		for(
			t = pool_iter_next( iter );
			t;
			t = pool_iter_next( iter )
		)	{
			printf( "%s\n", t->s );
		}
	} while( 0 );
	pool_iter_free( &iter );
}
#endif	/* NOPE */

static	int
only_messages(
	const struct dirent *	de
)
{
	int			retval;

	retval = 0;
	do	{
		retval = !strncmp( de->d_name, "messages", 8 );
	} while( 0 );
	return( retval );
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
	/* Setup the pools we'll be using				 */
	triggers = pool_new( sizeof(trigger_t), NULL, NULL );
	entries  = pool_new( sizeof(entry_t), NULL, NULL );
	ignores  = pool_new( sizeof(trigger_t), NULL, NULL );
	/* Process command line						 */
	while( (c = getopt( argc, argv, "Xa:A:cd:G:gi:I:lmnNO:o:rst:w:W:vy:" )) != EOF ) {
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
			xprintf_set_debug( debug );
			break;
		case 'a':
			add_pattern( triggers, optarg );
			break;
		case 'A':
			bulk_load( triggers, optarg );
			break;
		case 'c':
			colorize = 1;
			mark_entries = 1;
			break;
		case 'd':
			{
				for(
					date_kind = 0;
					date_kind < Ntimekinds;
					++date_kind
				)	{
					if( !strcasecmp(
						optarg,
						timekinds[date_kind].name
					) )	{
						break;
					}
				}
				if( date_kind >= Ntimekinds )	{
					date_kind = 0;
				}
			}
			break;
		case 'g':
			do_gap = 1;
			mark_entries = 1;
			break;
		case 'G':
			gap_threshold = strtoul( optarg, NULL, 10 );
			break;
		case 'i':
			add_pattern( ignores, optarg );
			break;
		case 'I':
			bulk_load( ignores, optarg );
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
		case 'N':
			want_lineno = 1;
			break;
		case 'o':
			ofile = optarg;
			break;
		case 'O':
			delta_hours = strtol( optarg, NULL, 0 );
			break;
		case 'r':
			show_rules = 1;
			mark_entries = 1;
			break;
		case 's':
			do_stats += 1;
			break;
		case 't':
			thumb = optarg;
			break;
		case 'v':
			printf(
				"%s %s\n",
				me,
				VERSION
#if	HAVE_LIBPCRE
				" with PCRE"
#endif	/* HAVE_LIBPCRE */
			);
			exit(0);
			/*NOTREACHED*/
		case 'w':
			about = optarg;
			break;
		case 'W':
			{
				char *	eos;
				window = (time_t) strtoul(
					optarg,
					&eos,
					10
				);
				switch( tolower( *eos & 0xFF ) )	{
				default:
					fprintf(
						stderr,
						"%s: bad scaler '%s'.\n",
						me,
						eos
					);
					exit(1);
					/*NOTREACHED			 */
				case 'w':
					window *= 7;
				case 'h':
					window *= 60;
				case 'm':
					window *= 60;
				case 's':
				case '\0':
					break;
				}
			}
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
		year = tm->tm_year;
	}
	/* Figure out about when the incident was supposed to happen	 */
	if( about )	{
		if( ! calc_timestamp( about, &incident ) )	{
			fprintf(
				stderr,
				"%s: illegal incident time '%s'.\n",
				me,
				optarg
			);
			exit(1);
		}
		if( !window )	{
			/* Default window is one day		 */
			window = (time_t) (24*60*60);
		}
	}
	/* Compile internal triggers unless we are forbidden		 */
	if( load_builtin_rules )	{
		char const * const *	builtin;

		for( builtin = builtin_triggers; *builtin; ++builtin )	{
			/* Skip deleted builtin rules			 */
			if( *builtin != (char *) -1 )	{
				add_pattern( triggers, *builtin );
			}
		}
	}
	/* List known triggers if asked					 */
	if( list_triggers )	{
		pool_iter_t *	iter;
		trigger_t *	t;

		iter = pool_iter_new( triggers );
		for(
			t = pool_iter_next( iter );
			t;
			t = pool_iter_next( iter )
		)	{
			puts( t->s );
		}
		pool_iter_free( &iter );
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
		/* Nothing specified, read "/var/log/messages*"		 */
		static char const	vdir[] = "/var/log";
		int			nfiles;
		struct dirent * *	namelist;
		int			i;

		nfiles = scandir(
			vdir,
			&namelist,
			only_messages,
			versionsort
		);
		if( nfiles == -1)	{
			perror( vdir );
			abort();
		}
		xprintf( 1, "%d message files", nfiles );
		for( i = 0; i < nfiles; ++i )	{
			struct dirent *	de = namelist[i];

			xprintf( 2, "%s", de->d_name );
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
			free( namelist[i] );
		}
		free( namelist );
	}
	/* Sort retained entries					 */
	flatten_and_sort_entries();
	/* Post-processing phase					 */
	if( mark_entries )	{
		announce_ignored();
		post_process();
	}
	/* Print results						 */
	print_entries();
	/* Want stats?							 */
	if( do_stats )	{
		static char const	fmt[] = "%15lu %s.\n";

		fprintf( stderr, fmt, log_stats.read,	"entries read" );
		fprintf( stderr, fmt, log_stats.poorly_formed, "poorly-formed entries" );
		fprintf( stderr, fmt, log_stats.dropped, "entries dropped" );
		fprintf( stderr, fmt, log_stats.flattened, "entries kept" );
		fprintf( stderr, fmt, log_stats.output,	"entries output" );
	}
	/* Get out of Dodge						 */
	return( nonfatal ? 1 : 0 );
}
