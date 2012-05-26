#ifndef	_POST_PROCESS_H
#define	_POST_PROCESS_H

#include <gcc-compat.h>

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

static	char const *	me = "vlm_tool";
static	unsigned	nonfatal;
static	char const *	thumb = "-";
static	char *		no_thumb;
static	unsigned	list_triggers;
static	unsigned	mark_entries;
static	unsigned	load_builtin_rules = 1;
static	char const *	ofile;
static	unsigned	show_rules;
static	unsigned	colorize;
static	pool_t *	triggers;
static	int		year;
static	unsigned	hosts_qty;
static	unsigned	hostsPos;
static	char * *	hosts;
static	pool_t *	entries;
static	size_t		entries_qty;
static	entry_t * *	flat_entries;
static	unsigned	debug;
static	size_t		hlen;
static	pool_t *	ignores;
static	int		want_lineno;

static char const	sgr_red[] =	{
	"\033[1;31;22;47m"		/* Bright red text, dirty white bg */
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
		{ "May\0" },
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

	/* Compile the recognition pattern				 */
	xprintf( 3, "Adding rule '%s'", rule );
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
		/* Qualify the line					 */
		iter = pool_iter_new( ignores );
		fired = NULL;
		do	{
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
					/* Leave loop early		 */
					break;
				}
			}
		} while( 0 );
		pool_iter_free( &iter );
		if( fired )	{
			/* Do not want this line at all			 */
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
	struct tm *		tm;

	/* First, the thumb (if marked)					 */
	if( mark_entries )	{
		printf(
			"%s ",
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

extern	void		post_process( void );

#endif	/* _POST_PROCESS_H */
