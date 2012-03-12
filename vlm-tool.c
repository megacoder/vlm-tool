#define	_GNU_SOURCE

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
#include <entries.h>
#include <pool.h>

static	char const *	me = "vlm_tool";
static	unsigned	nonfatal;
static	char const *	thumb = "-";
static	char *		no_thumb;
static	unsigned	list_triggers;
static	unsigned	mark_entries;
static	unsigned	load_builtin_rules = 1;
static	char const *	ofile;
static	unsigned	show_rules;
static	unsigned	show_stats;
static	unsigned	colorize;
static	pool_t *	triggers;
static	int		year;
static	unsigned	hostsQty;
static	unsigned	hostsPos;
static	char * *	hosts;
static	pool_t *	entries;
static	size_t		entries_qty;
static	entry_t * *	flat_entries;
static	unsigned	debug;
static	size_t		hlen;

static char const	sgr_red[] =	{
	"\033[1;31m"
};
static char const	sgr_reset[] =	{
	"\033[0m"
};

static	void *		_inline
xmalloc(
	size_t		size
)
{
	void * const	retval = malloc( size );

	if( !retval )	{
		perror( "out of memory" );
		abort();
	}
	return( retval );
}

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
			static	size_t const	incr = 32;

			hostsQty += incr;
			hosts = realloc(
				hosts,
				sizeof(hosts[0]) * hostsQty
			);
		}
		retval = hostsPos++;
		hosts[ retval ] = xstrdup( name );
		hlen = max(hlen, strlen( name ));
	} while( 0 );
	return( retval );
}

static	void
add_trigger(
	char const * const	rule
)
{
	trigger_t * const	t = pool_alloc( triggers );

	/* Compile the resulting trigger				 */
	if( debug > 0 )	{
		printf( "Adding rule '%s'.\n", rule );
	}
	t->s = rule;
	if( regcomp(
		&(t->re),
		t->s,
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
		add_trigger( xstrdup( bp ) );
	}
	fclose( fyle );
}

#if	0
static	void	_inline
do_match(
	entry_t *		old_e,
	regmatch_t *		matches
)
{
	regmatch_t * const	mid = matches+0;
	char			color_buffer[ BUFSIZ * 3 ];

	e->trigger = t;
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
	if( (e->trigger != NULL) | mark_entries )	{
		e->resid = xstrdup( resid );
		time_t		timestamp;
		unsigned	host_id;
		char *		resid;
		trigger_t *	trigger;
		add_entry( e );
	}
}
#endif	/* NOPE */

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
		trigger_t *	fired;
		int		keep;
		pool_iter_t *	iter;

		/* Drop trailing whitespace				 */
		for( bp = buf + l; (bp > buf) && isspace(bp[-1]); --bp ) {
			bp[-1] = '\0';
		}
		/* Isolate timestamp, hostname, and resid		 */
		ts = buf;
		buf[15] = '\0';
		host = buf + 16;
		for( resid = host+1; *resid && !isspace( *resid ); ++resid );
		if( *resid )	{
			(*resid++) = '\0';
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
				regmatch_t		matches[ 10 ];
				if( !regexec(
					&fired->re,
					resid,
					DIM(matches),
					matches,
					0
				) )	{
					keep  = 1;
					break;
				}
			}
		} while( 0 );
		pool_iter_free( &iter );
		/* Remember this entry if we're keeping them		 */
		if( keep )	{
			entry_t * const	e = pool_alloc( entries );

			e->host_id = add_host( host );

			/* Convert info to timestamp			 */
			if(calc_timestamp( ts, &(e->timestamp) ) ) {
				/* Bad date, show it first in list	 */
				memset(
					&(e->timestamp),
					0,
					sizeof(e->timestamp)
				);
			}
			e->trigger = fired;
			e->resid = xstrdup( resid );
			entries_qty += 1;
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
	iter         = pool_iter_new( entries );
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
		fprintf(
			stderr,
			"Sorting %u entries.\n",
			(unsigned) entries_qty
		);
		qsort(
			flat_entries,
			entries_qty,
			sizeof(flat_entries[0]),
			compar
		);
	} while( 0 );
	pool_iter_free( &iter );
}

static	void
sgr_host(
	unsigned	host_id
)
{
	printf( "\033[1;%dm", (31 + (host_id % 6)) );
}

static	void
print_one_entry(
	entry_t * const		e
)
{
	static const char	fmt[] = "%-*s ";
	struct tm *		tm;

	/* First, the thumb (if marked)				 */
	if( mark_entries )	{
		printf(
			"%s ",
			e->trigger == NULL ? no_thumb : thumb
		);
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
	/* FIXME: add colorizing codes to 'resid' 		 */
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

			asprintf(
				&e->resid,
				"%.*s" "%s%.*s%s" "%s",
				rm->rm_so,
				resid,
				sgr_red,
				rm->rm_eo - rm->rm_so,
				resid + rm->rm_so,
				sgr_reset,
				resid + rm->rm_eo
			);
			free( resid );
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
	for( i = 0; i < entries_qty; ++i )	{
		entry_t * const		e = flat_entries[i];

		print_one_entry( e );
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

static	void
post_process(
	void
)
{
	static char const * const	start_strings[] =	{
		"unable to handle",
		"call trace:"
	};
	static	size_t			Nstart_strings =
		DIM( start_strings );
	static	trigger_t *		starters;
	static	trigger_t		ender;
	size_t				i;

	/* We ain't got nuthin' yet					 */
	starters = xmalloc( Nstart_strings * sizeof(starters[0]) );
	for( i = 0; i < Nstart_strings; ++i )	{
		trigger_t * const	t = starters + i;

		t->s = start_strings[ i ];
		if( regcomp(
			&(t->re),
			t->s,
			(REG_EXTENDED|REG_ICASE)
		) )	{
			perror( "out of starter memory" );
			abort();
		}
	}
	ender.s = "kernel:";
	if( regcomp(
		&(ender.re),
		ender.s,
		(REG_EXTENDED|REG_ICASE)
	) )	{
		perror( "out of ender memory" );
		abort();
	}
	/* Iterate over all the entries, looking for a starter		 */
	for( i = 0; i < entries_qty; ++i )	{
#if	0
		entry_t * const		e = flat_entries[i];
#endif	/* NOPE */
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
	/* Setup the pools we'll be using				 */
	triggers = pool_new( sizeof(trigger_t), NULL, NULL );
	entries  = pool_new( sizeof(entry_t), NULL, NULL );
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
	flatten_and_sort_entries();
	/* Post-processing phase					 */
	if( mark_entries )	{
		post_process();
	}
	/* Print results						 */
	print_entries();
	/* Get out of Dodge						 */
	return( nonfatal ? 1 : 0 );
}
