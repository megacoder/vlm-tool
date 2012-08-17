#ifndef	_X_FUNCS_H
#define	_X_FUNCS_H

#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include <gcc-compat.h>

static	void *		_used
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

static	void *		_used
xstrdup(
	char const * const	s
)
{
	void * const	retval = strdup( s );

	if( !retval )	{
		fprintf( stderr, "out of xstrdup memory.\n" );
		exit(1);
	}
	return( retval );
}

#endif	/* _X_FUNCS_H */
